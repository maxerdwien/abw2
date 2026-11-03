// Scripted test harness for alaskan cosmobear spacefighting.
//
// Links the real ship classes (everything except main.cpp), drives them with scripted
// controller input, replicates the in-game ship physics loop, can dump PNG screenshots
// of the rendered scene, and checks expectations. Exit code is nonzero if any check fails.
//
// Usage:  harness.exe [--debug] [--sound] [--shots <dir>] <script.txt> [more scripts...]
//
// Script language (one command per line, '#' starts a comment):
//   ship <slot 0-3> <black|grizzly|polar|panda|kodiak> <x_px> <y_px> [ally_slot]
//   asteroid <x_px> <y_px> [vx_px_per_frame] [vy_px_per_frame]
//   move <slot> <dx> <dy>          left stick, each -1..1 (0 0 releases)
//   aim <slot> <dx> <dy>           right stick, each -1..1
//   fire1|fire2|fire3 <slot> <0|1> hold / release RB, RT, LB
//   boost <slot>                   tap speed boost
//   item <slot> <shield|laser_sights|speed_up|bounce|small_pwrup|bullet_bounce> <frames>
//   teleport <slot> <x_px> <y_px>  place a ship (zeroes its velocity)
//   wind <angle_deg> <strength>    steady wind, as on the nome stage (0 deg = +x; strength in
//                                  velocity units per frame, the stage peaks at 4000; 0 = calm)
//   run <frames>                   advance the simulation
//   shot <file.png>                render the scene and save it
//   print <slot>                   dump a ship's state
//   debug <0|1>                    toggle hitbox rendering for screenshots
//   expect <slot> <field> <op> <value>
//        field: x y vx vy percent lives stamina damage_done damage_taken kills invincibility
//        op:    <  <=  >  >=  ==  !=       (x/y in px, vx/vy in px per frame)
//   expect_dist <slot_a> <slot_b> <op> <value_px>
//
// Everything runs from the exe's own directory so the game's "..\Project1\assets" paths resolve.

#define SDL_MAIN_HANDLED
#include <windows.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "spaceship.h"
#include "grizzly.h"
#include "black.h"
#include "polar.h"
#include "panda.h"
#include "kodiak.h"

#include "item.h"
#include "asteroid.h"
#include "renderer.h"
#include "wind.h"

// arena constants, mirrored from main.cpp
static const int WIDTH_UNITS = 10000 * 1280;
static const int HEIGHT_UNITS = 10000 * 720;
static const int STATUS_BAR_WIDTH = 10000 * 150;
static const int DEAD_ZONE = 5000;

static const int WINDOW_W = 1280;
static const int WINDOW_H = 720;

static SDL_Window* window = NULL;
static SDL_Renderer* sdl_renderer = NULL;
static SDL_Texture* render_target = NULL;
static SDL_Texture* bg_tex = NULL;
static Renderer* r = NULL;

static std::string original_cwd;
static std::string shot_dir;
static bool opt_debug = false;
static bool opt_sound = false;

static int checks_total = 0;
static int checks_failed = 0;

struct PlayerInput {
	int l_x = 0, l_y = 0;
	int r_x = 0, r_y = 0;
	bool fire1 = false, fire2 = false, fire3 = false;
	bool boost = false;
};

struct World {
	Ship* ships[4] = { NULL, NULL, NULL, NULL };
	std::string ship_names[4];
	PlayerInput input[4];
	Asteroid* asteroids[100];
	int num_asteroids = 0;
	SDL_Haptic* haptics[4] = { NULL, NULL, NULL, NULL };
	int frame = 0;
	Wind* wind = NULL;

	void clear() {
		for (int i = 0; i < 4; i++) {
			delete ships[i];
			ships[i] = NULL;
			ship_names[i].clear();
			input[i] = PlayerInput();
		}
		for (int i = 0; i < num_asteroids; i++) delete asteroids[i];
		if (wind) {
			wind->reset();
			wind->strength = 0;
		}
		num_asteroids = 0;
		frame = 0;
	}

	void step();
};

static Ship* make_ship(const std::string& type, int slot, int ally, int x, int y) {
	if (type == "black")   return new Black(slot, ally, -1, x, y, r);
	if (type == "grizzly") return new Grizzly(slot, ally, -1, x, y, r);
	if (type == "polar")   return new Polar(slot, ally, -1, x, y, r);
	if (type == "panda")   return new Panda(slot, ally, -1, x, y, r);
	if (type == "kodiak")  return new Kodiak(slot, ally, -1, x, y, r);
	return NULL;
}

// One frame of the in-game update loop (main.cpp, in_game state), minus items on the
// field, networking and the countdown. Order of operations matches the game.
void World::step() {
	for (int i = 0; i < 4; i++) {
		Ship* ship = ships[i];
		if (!ship) continue;
		if (ship->lives == 0) continue;

		PlayerInput& in = input[i];

		double left_mag = sqrt(pow(in.l_x, 2) + pow(in.l_y, 2));
		if (left_mag > DEAD_ZONE) {
			ship->move_dir_x = in.l_x;
			ship->move_dir_y = in.l_y;
			ship->face_dir_x = ship->move_dir_x;
			ship->face_dir_y = ship->move_dir_y;
		} else {
			ship->move_dir_x = 0;
			ship->move_dir_y = 0;
		}

		double right_mag = sqrt(pow(in.r_x, 2) + pow(in.r_y, 2));
		if (right_mag > DEAD_ZONE) {
			ship->desired_gun_dir_x = in.r_x;
			ship->desired_gun_dir_y = in.r_y;
		}

		ship->do_fire_1 = in.fire1;
		ship->do_fire_2 = in.fire2;
		ship->do_fire_3 = in.fire3;
		if (in.boost) {
			ship->do_speed_boost = true;
			in.boost = false;
		}

		ship->update();

		if (ship->invincibility_cooldown > 0) {
			ship->invincibility_cooldown--;
		}

		ship->stamina += ship->stamina_per_frame;
		if (ship->stamina > ship->stamina_max) {
			ship->stamina = ship->stamina_max;
		}

		ship->fire_1();
		ship->fire_2();
		ship->fire_3();

		for (int j = 0; j < NUM_ITEM_TYPES; j++) {
			if (ship->item_times[j] > 0) ship->item_times[j]--;
		}

		if (ship->item_times[item_type::small_pwrup] > 60) {
			ship->radius = ship->normal_radius / 2;
			ship->gun_length = ship->normal_gun_length / 2;
		} else if (ship->item_times[item_type::small_pwrup] > 0) {
			if (ship->item_times[item_type::small_pwrup] % 15 < 7) {
				ship->radius = ship->normal_radius;
				ship->gun_length = ship->normal_gun_length;
			} else {
				ship->radius = ship->normal_radius / 2;
				ship->gun_length = ship->normal_gun_length / 2;
			}
		} else {
			ship->radius = ship->normal_radius;
			ship->gun_length = ship->normal_gun_length;
		}

		if (ship->speed_boost_cooldown > 0) {
			ship->speed_boost_cooldown--;
		}

		if (ship->invincibility_cooldown < 2 * 60) {
			double accel_mag = sqrt(pow(ship->move_dir_x, 2) + pow(ship->move_dir_y, 2));
			if (accel_mag > ship->max_accel) {
				accel_mag = ship->max_accel;
			}
			if (ship->do_speed_boost && ship->speed_boost_cooldown == 0) {
				accel_mag = ship->max_accel * 40;
				ship->speed_boost_cooldown += ship->speed_boost_delay;
				ship->do_speed_boost = false;
			}
			if (ship->item_times[speed_up] > 0) {
				accel_mag *= 1.5;
			}

			if (ship->move_dir_x != 0 || ship->move_dir_y != 0) {
				double m = sqrt(pow(ship->move_dir_x, 2) + pow(ship->move_dir_y, 2));
				ship->x_accel = (int)(accel_mag * ship->move_dir_x / m);
				ship->y_accel = (int)(accel_mag * ship->move_dir_y / m);
			} else {
				ship->x_accel = 0;
				ship->y_accel = 0;
			}

			ship->x_vel += ship->x_accel;
			ship->y_vel += ship->y_accel;

			wind->push(ship);

			// friction
			int iterations_per_frame = 40;
			for (int j = 0; j < iterations_per_frame; j++) {
				double total_vel = sqrt(pow(ship->x_vel, 2) + pow(ship->y_vel, 2));
				double friction_accel = ((pow(total_vel, 2) + ship->constant_friction) / ship->friction_limiter) / iterations_per_frame;

				int friction_accel_x = (int)(((double)friction_accel * ship->x_vel) / total_vel);
				if (ship->x_vel > 0) {
					ship->x_vel -= friction_accel_x;
					if (ship->x_vel < 0) ship->x_vel = 0;
				} else if (ship->x_vel < 0) {
					ship->x_vel -= friction_accel_x;
					if (ship->x_vel > 0) ship->x_vel = 0;
				}

				int friction_accel_y = (int)(((double)friction_accel * ship->y_vel) / total_vel);
				if (ship->y_vel > 0) {
					ship->y_vel -= friction_accel_y;
					if (ship->y_vel < 0) ship->y_vel = 0;
				} else if (ship->y_vel < 0) {
					ship->y_vel -= friction_accel_y;
					if (ship->y_vel > 0) ship->y_vel = 0;
				}
			}

			// ship-ship collisions
			for (int j = i + 1; j < 4; j++) {
				if (!ships[j]) continue;
				if (ships[j]->lives == 0) continue;
				double dist = sqrt(pow(ship->x_pos - ships[j]->x_pos, 2) + pow(ship->y_pos - ships[j]->y_pos, 2));
				if (dist == 0) dist = 1;
				if (dist <= (ships[i]->radius + ships[j]->radius)) {
					double total_force = 140000000000000000.0 / pow(dist, 2);
					double x_force = (ship->x_pos - ships[j]->x_pos) * total_force / dist;
					ship->x_vel += (int)(x_force / ship->weight);
					ships[j]->x_vel -= (int)(x_force / ship->weight);
					double y_force = (ship->y_pos - ships[j]->y_pos) * total_force / dist;
					ship->y_vel += (int)(y_force / ship->weight);
					ships[j]->y_vel -= (int)(y_force / ship->weight);
				}
			}
		}

		// asteroid collisions
		for (int j = 0; j < num_asteroids; j++) {
			Asteroid* a = asteroids[j];
			double dist = sqrt(pow(ship->x_pos - a->x_pos, 2) + pow(ship->y_pos - a->y_pos, 2));
			if (dist < (ship->radius + a->radius)) {
				ship->take_knockback(ship->x_pos - a->x_pos, ship->y_pos - a->y_pos, 100, 20, 28, haptics[i]);
			}
		}

		ship->x_pos += ship->x_vel;
		ship->y_pos += ship->y_vel;

		// death
		int screen_edge_wiggle_room = (int)(ship->radius * 1.5);
		if (ship->x_pos < (STATUS_BAR_WIDTH - screen_edge_wiggle_room) || ship->x_pos > (WIDTH_UNITS + screen_edge_wiggle_room)
			|| ship->y_pos < (0 - screen_edge_wiggle_room) || ship->y_pos > (HEIGHT_UNITS + screen_edge_wiggle_room)) {
			if (ship->item_times[bounce] > 0 || ship->invincibility_cooldown > 0) {
				if (ship->x_pos < STATUS_BAR_WIDTH && ship->x_vel < 0) {
					ship->x_vel *= -1;
				} else if (ship->x_pos > WIDTH_UNITS && ship->x_vel > 0) {
					ship->x_vel *= -1;
				} else if (ship->y_pos < 0 && ship->y_vel < 0) {
					ship->y_vel *= -1;
				} else if (ship->y_pos > HEIGHT_UNITS && ship->y_vel > 0) {
					ship->y_vel *= -1;
				}
			} else {
				ship->lives--;

				int last_hit = ship->last_hit;
				if (last_hit != -1 && ships[last_hit]) {
					ships[last_hit]->kills[ships[last_hit]->num_kills] = i;
					ships[last_hit]->num_kills++;
					if (ship->id == last_hit) {
						ship->num_suicides++;
					}
				}

				printf("  [frame %d] ship %d (%s) died, %d lives left\n", frame, i, ship_names[i].c_str(), ship->lives);

				if (ship->lives <= 0) {
					ship->die();
					ship->lives = 0;
				}

				ship->invincibility_cooldown += ship->respawn_invincibility_delay;

				ship->x_pos = ship->respawn_x;
				ship->y_pos = ship->respawn_y;
				ship->x_vel = 0;
				ship->y_vel = 0;
			}
		}
	}

	for (int i = 0; i < 4; i++) {
		Ship* ship = ships[i];
		if (!ship) continue;
		ship->update_projectiles_1(STATUS_BAR_WIDTH, WIDTH_UNITS, 0, HEIGHT_UNITS, ships, asteroids, num_asteroids, haptics);
		ship->update_projectiles_3(STATUS_BAR_WIDTH, WIDTH_UNITS, 0, HEIGHT_UNITS, ships, asteroids, num_asteroids, haptics);
		ship->update_projectiles_2(STATUS_BAR_WIDTH, WIDTH_UNITS, 0, HEIGHT_UNITS, ships, asteroids, num_asteroids, haptics);
	}

	for (int i = 0; i < num_asteroids; i++) {
		asteroids[i]->update();
	}

	frame++;
}

// ---------------------------------------------------------------- rendering

static void render_world(World& w) {
	r->render_solid_bg();
	if (bg_tex) {
		r->render_texture(bg_tex, WIDTH_UNITS / 2, HEIGHT_UNITS / 2, 0, 1.02);
	}

	for (int i = 0; i < 4; i++) {
		if (!w.ships[i]) continue;
		if (w.ships[i]->lives == 0) continue;
		w.ships[i]->render();
	}
	for (int i = 0; i < 4; i++) {
		if (!w.ships[i]) continue;
		w.ships[i]->render_projectiles_1();
		w.ships[i]->render_projectiles_2();
		w.ships[i]->render_projectiles_3();
	}
	for (int i = 0; i < w.num_asteroids; i++) {
		w.asteroids[i]->render();
	}
	w.wind->render();
}

static std::string resolve_output_path(const std::string& p) {
	bool absolute = p.size() > 1 && (p[1] == ':' || p[0] == '\\' || p[0] == '/');
	if (absolute) return p;
	std::string base = shot_dir.empty() ? original_cwd : shot_dir;
	return base + "\\" + p;
}

static bool save_screenshot(World& w, const std::string& file) {
	render_world(w);

	SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_W, WINDOW_H, 32, SDL_PIXELFORMAT_ARGB8888);
	if (!surf) {
		printf("  screenshot: SDL_CreateRGBSurfaceWithFormat failed: %s\n", SDL_GetError());
		return false;
	}
	if (SDL_RenderReadPixels(sdl_renderer, NULL, SDL_PIXELFORMAT_ARGB8888, surf->pixels, surf->pitch) != 0) {
		printf("  screenshot: SDL_RenderReadPixels failed: %s\n", SDL_GetError());
		SDL_FreeSurface(surf);
		return false;
	}
	std::string path = resolve_output_path(file);
	int rc = IMG_SavePNG(surf, path.c_str());
	SDL_FreeSurface(surf);
	if (rc != 0) {
		printf("  screenshot: IMG_SavePNG failed for %s: %s\n", path.c_str(), IMG_GetError());
		return false;
	}
	printf("  [frame %d] screenshot -> %s\n", w.frame, path.c_str());
	return true;
}

// ---------------------------------------------------------------- script

static double px(int units) { return units / 10000.0; }

static void print_ship(World& w, int slot) {
	Ship* s = w.ships[slot];
	if (!s) { printf("  ship %d: (none)\n", slot); return; }
	printf("  [frame %d] ship %d %-8s x=%.1f y=%.1f vx=%.2f vy=%.2f pct=%d lives=%d stam=%d/%d dmg_done=%d dmg_taken=%d kills=%d inv=%d\n",
		w.frame, slot, w.ship_names[slot].c_str(), px(s->x_pos), px(s->y_pos), px(s->x_vel), px(s->y_vel),
		s->percent, s->lives, s->stamina, s->stamina_max, s->damage_done, s->damage_taken, s->num_kills, s->invincibility_cooldown);
}

static bool get_field(Ship* s, const std::string& f, double& out) {
	if (f == "x") out = px(s->x_pos);
	else if (f == "y") out = px(s->y_pos);
	else if (f == "vx") out = px(s->x_vel);
	else if (f == "vy") out = px(s->y_vel);
	else if (f == "percent") out = s->percent;
	else if (f == "lives") out = s->lives;
	else if (f == "stamina") out = s->stamina;
	else if (f == "damage_done") out = s->damage_done;
	else if (f == "damage_taken") out = s->damage_taken;
	else if (f == "kills") out = s->num_kills;
	else if (f == "invincibility") out = s->invincibility_cooldown;
	else return false;
	return true;
}

static bool compare(double a, const std::string& op, double b) {
	if (op == "<") return a < b;
	if (op == "<=") return a <= b;
	if (op == ">") return a > b;
	if (op == ">=") return a >= b;
	if (op == "==") return a == b;
	if (op == "!=") return a != b;
	return false;
}

static void check(bool ok, const std::string& what, double actual, World& w) {
	checks_total++;
	if (ok) {
		printf("  [frame %d] PASS  %s   (actual %.2f)\n", w.frame, what.c_str(), actual);
	} else {
		checks_failed++;
		printf("  [frame %d] FAIL  %s   (actual %.2f)\n", w.frame, what.c_str(), actual);
	}
}

static bool parse_item(const std::string& s, item_type& out) {
	if (s == "shield") out = shield;
	else if (s == "laser_sights") out = laser_sights;
	else if (s == "speed_up") out = speed_up;
	else if (s == "bounce") out = bounce;
	else if (s == "small_pwrup") out = small_pwrup;
	else if (s == "bullet_bounce") out = bullet_bounce;
	else return false;
	return true;
}

static int stick(double v) {
	if (v > 1) v = 1;
	if (v < -1) v = -1;
	return (int)(v * 32767);
}

static bool valid_slot(World& w, int slot, const std::string& cmd, int line_no) {
	if (slot < 0 || slot > 3 || !w.ships[slot]) {
		printf("  line %d: %s: no ship in slot %d\n", line_no, cmd.c_str(), slot);
		checks_total++;
		checks_failed++;
		return false;
	}
	return true;
}

static bool run_script(const std::string& path) {
	std::ifstream in(path);
	if (!in) {
		printf("cannot open script %s\n", path.c_str());
		checks_total++;
		checks_failed++;
		return false;
	}
	printf("=== %s\n", path.c_str());

	World w;
	w.wind = new Wind(r, STATUS_BAR_WIDTH, WIDTH_UNITS, 0, HEIGHT_UNITS);
	w.wind->strength = 0;
	std::string line;
	int line_no = 0;
	while (std::getline(in, line)) {
		line_no++;
		size_t hash = line.find('#');
		if (hash != std::string::npos) line = line.substr(0, hash);
		std::istringstream ss(line);
		std::vector<std::string> t;
		std::string tok;
		while (ss >> tok) t.push_back(tok);
		if (t.empty()) continue;

		const std::string& cmd = t[0];
		auto need = [&](size_t n) {
			if (t.size() < n) {
				printf("  line %d: %s needs %d arguments\n", line_no, cmd.c_str(), (int)n - 1);
				checks_total++;
				checks_failed++;
				return false;
			}
			return true;
		};

		if (cmd == "ship") {
			if (!need(5)) continue;
			int slot = atoi(t[1].c_str());
			int ally = t.size() > 5 ? atoi(t[5].c_str()) : -1;
			if (slot < 0 || slot > 3) { printf("  line %d: bad slot\n", line_no); continue; }
			delete w.ships[slot];
			w.input[slot] = PlayerInput();   // a fresh ship starts with nothing held
			w.ships[slot] = make_ship(t[2], slot, ally, (int)(atof(t[3].c_str()) * 10000), (int)(atof(t[4].c_str()) * 10000));
			if (!w.ships[slot]) {
				printf("  line %d: unknown ship type '%s'\n", line_no, t[2].c_str());
				checks_total++;
				checks_failed++;
				continue;
			}
			w.ship_names[slot] = t[2];
			// start the way the game does: aiming right, no spawn invincibility flicker in shots
			w.ships[slot]->gun_dir_x = 1;
			w.ships[slot]->gun_dir_y = 0;
			w.ships[slot]->invincibility_cooldown = 0;
			printf("  ship %d = %s at (%s, %s)\n", slot, t[2].c_str(), t[3].c_str(), t[4].c_str());
		} else if (cmd == "asteroid") {
			if (!need(3)) continue;
			int vx = t.size() > 4 ? (int)(atof(t[3].c_str()) * 10000) : 0;
			int vy = t.size() > 4 ? (int)(atof(t[4].c_str()) * 10000) : 0;
			if (w.num_asteroids < 100) {
				w.asteroids[w.num_asteroids++] = new Asteroid((int)(atof(t[1].c_str()) * 10000), (int)(atof(t[2].c_str()) * 10000), vx, vy, r);
			}
		} else if (cmd == "wind") {
			if (!need(3)) continue;
			w.wind->angle = atof(t[1].c_str()) * 3.14159265358979323846 / 180.0;
			w.wind->strength = atoi(t[2].c_str());
		} else if (cmd == "move" || cmd == "aim") {
			if (!need(4)) continue;
			int slot = atoi(t[1].c_str());
			if (!valid_slot(w, slot, cmd, line_no)) continue;
			int x = stick(atof(t[2].c_str()));
			int y = stick(atof(t[3].c_str()));
			if (cmd == "move") { w.input[slot].l_x = x; w.input[slot].l_y = y; }
			else { w.input[slot].r_x = x; w.input[slot].r_y = y; }
		} else if (cmd == "fire1" || cmd == "fire2" || cmd == "fire3") {
			if (!need(3)) continue;
			int slot = atoi(t[1].c_str());
			if (!valid_slot(w, slot, cmd, line_no)) continue;
			bool on = atoi(t[2].c_str()) != 0;
			if (cmd == "fire1") w.input[slot].fire1 = on;
			else if (cmd == "fire2") w.input[slot].fire2 = on;
			else w.input[slot].fire3 = on;
		} else if (cmd == "boost") {
			if (!need(2)) continue;
			int slot = atoi(t[1].c_str());
			if (!valid_slot(w, slot, cmd, line_no)) continue;
			w.input[slot].boost = true;
		} else if (cmd == "item") {
			if (!need(4)) continue;
			int slot = atoi(t[1].c_str());
			if (!valid_slot(w, slot, cmd, line_no)) continue;
			item_type it;
			if (!parse_item(t[2], it)) { printf("  line %d: unknown item '%s'\n", line_no, t[2].c_str()); continue; }
			w.ships[slot]->item_times[it] += atoi(t[3].c_str());
		} else if (cmd == "teleport") {
			if (!need(4)) continue;
			int slot = atoi(t[1].c_str());
			if (!valid_slot(w, slot, cmd, line_no)) continue;
			w.ships[slot]->x_pos = (int)(atof(t[2].c_str()) * 10000);
			w.ships[slot]->y_pos = (int)(atof(t[3].c_str()) * 10000);
			w.ships[slot]->x_vel = 0;
			w.ships[slot]->y_vel = 0;
		} else if (cmd == "run") {
			if (!need(2)) continue;
			int n = atoi(t[1].c_str());
			for (int i = 0; i < n; i++) w.step();
		} else if (cmd == "shot") {
			if (!need(2)) continue;
			if (!save_screenshot(w, t[1])) { checks_total++; checks_failed++; }
		} else if (cmd == "print") {
			if (!need(2)) continue;
			int slot = atoi(t[1].c_str());
			if (!valid_slot(w, slot, cmd, line_no)) continue;
			print_ship(w, slot);
		} else if (cmd == "debug") {
			if (!need(2)) continue;
			r->render_debug = atoi(t[1].c_str()) != 0;
		} else if (cmd == "expect") {
			if (!need(5)) continue;
			int slot = atoi(t[1].c_str());
			if (!valid_slot(w, slot, cmd, line_no)) continue;
			double actual;
			if (!get_field(w.ships[slot], t[2], actual)) {
				printf("  line %d: unknown field '%s'\n", line_no, t[2].c_str());
				checks_total++;
				checks_failed++;
				continue;
			}
			double want = atof(t[4].c_str());
			check(compare(actual, t[3], want), "ship " + t[1] + " " + t[2] + " " + t[3] + " " + t[4], actual, w);
		} else if (cmd == "expect_dist") {
			if (!need(5)) continue;
			int a = atoi(t[1].c_str());
			int b = atoi(t[2].c_str());
			if (!valid_slot(w, a, cmd, line_no) || !valid_slot(w, b, cmd, line_no)) continue;
			double dist = px((int)sqrt(pow((double)w.ships[a]->x_pos - w.ships[b]->x_pos, 2) + pow((double)w.ships[a]->y_pos - w.ships[b]->y_pos, 2)));
			double want = atof(t[4].c_str());
			check(compare(dist, t[3], want), "dist(" + t[1] + "," + t[2] + ") " + t[3] + " " + t[4], dist, w);
		} else {
			printf("  line %d: unknown command '%s'\n", line_no, cmd.c_str());
			checks_total++;
			checks_failed++;
		}
	}

	w.clear();
	delete w.wind;
	w.wind = NULL;
	return true;
}

// ---------------------------------------------------------------- main

static std::string exe_dir() {
	char buf[MAX_PATH];
	GetModuleFileNameA(NULL, buf, MAX_PATH);
	std::string p(buf);
	size_t slash = p.find_last_of("\\/");
	return slash == std::string::npos ? "." : p.substr(0, slash);
}

static std::string full_path(const std::string& p) {
	char buf[MAX_PATH];
	if (GetFullPathNameA(p.c_str(), MAX_PATH, buf, NULL) == 0) return p;
	return std::string(buf);
}

int main(int argc, char* argv[]) {
	SDL_SetMainReady();

	std::vector<std::string> scripts;
	for (int i = 1; i < argc; i++) {
		std::string a = argv[i];
		if (a == "--debug") opt_debug = true;
		else if (a == "--sound") opt_sound = true;
		else if (a == "--shots" && i + 1 < argc) shot_dir = full_path(argv[++i]);
		else scripts.push_back(full_path(a));
	}
	if (scripts.empty()) {
		printf("usage: harness.exe [--debug] [--sound] [--shots <dir>] <script.txt> [more scripts...]\n");
		return 2;
	}

	{
		char buf[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, buf);
		original_cwd = buf;
	}
	if (!shot_dir.empty()) {
		CreateDirectoryA(shot_dir.c_str(), NULL);
	}

	// the game resolves assets relative to its exe directory
	SetCurrentDirectoryA(exe_dir().c_str());

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 2;
	}
	IMG_Init(IMG_INIT_PNG);

	bool audio = Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == 0;
	if (audio) {
		Mix_AllocateChannels(32);
		if (!opt_sound) Mix_Volume(-1, 0);
	} else {
		printf("note: no audio device (%s); sound calls become no-ops\n", Mix_GetError());
	}

	window = SDL_CreateWindow("harness", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_W, WINDOW_H, SDL_WINDOW_HIDDEN);
	if (!window) {
		printf("SDL_CreateWindow error: %s\n", SDL_GetError());
		return 2;
	}
	sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	if (!sdl_renderer) {
		sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE);
	}
	if (!sdl_renderer) {
		printf("SDL_CreateRenderer error: %s\n", SDL_GetError());
		return 2;
	}
	render_target = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, WINDOW_W, WINDOW_H);
	SDL_SetRenderTarget(sdl_renderer, render_target);

	r = new Renderer(sdl_renderer, WINDOW_W, WINDOW_H);
	r->render_normal = true;
	r->render_debug = opt_debug;

	bg_tex = r->LoadTexture("..\\Project1\\assets\\background.png");

	for (size_t i = 0; i < scripts.size(); i++) {
		run_script(scripts[i]);
	}

	printf("\n%d checks, %d failed\n", checks_total, checks_failed);

	if (bg_tex) SDL_DestroyTexture(bg_tex);
	delete r;
	SDL_DestroyTexture(render_target);
	SDL_DestroyRenderer(sdl_renderer);
	SDL_DestroyWindow(window);
	if (audio) Mix_CloseAudio();
	IMG_Quit();
	SDL_Quit();

	return checks_failed == 0 ? 0 : 1;
}

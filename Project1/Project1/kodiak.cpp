#include <string>
#include <SDL.h>
#include <SDL_mixer.h>

#include "serializer.h"

#include "asteroid.h"
#include "spaceship.h"
#include "spark.h"
#include "sawblade.h"
#include "kodiak.h"
#include "renderer.h"

#include "item.h"


Kodiak::Kodiak(int identifier, int a1, int a2, int x, int y, Renderer* rend) {
	id = identifier;

	ally1 = a1;
	ally2 = a2;

	x_pos = x;
	y_pos = y;

	respawn_x = x;
	respawn_y = y;

	stamina_max = 2000;
	stamina = stamina_max;
	stamina_per_frame = 5;

	max_accel = normal_max_accel;

	radius = 44 * 10000;
	normal_radius = radius;
	weight = 175;

	roar_channel = 24 + 2 * id;

	r = rend;

	for (int i = 0; i < 4; i++) {
		roar_hit[i] = false;
	}

	// roar ring is a white texture tinted to the team color
	Uint8 tint_r = 255, tint_g = 255, tint_b = 255;

	if (id == 0) {
		ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\kodiak-red.png");
		sawblade_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\sawbladeRed.png");
		harpoon_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\harpoonRed.png");
		tint_r = 255; tint_g = 70; tint_b = 70;
	} else if (id == 1) {
		if (ally1 == -1) {
			ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\kodiak-blue.png");
			sawblade_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\sawbladeBlue.png");
			harpoon_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\harpoonBlue.png");
			tint_r = 80; tint_g = 120; tint_b = 255;
		} else {
			ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\kodiak-magenta.png");
			sawblade_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\sawbladeMagenta.png");
			harpoon_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\harpoonMagenta.png");
			tint_r = 255; tint_g = 70; tint_b = 200;
		}
	} else if (id == 2) {
		if (ally1 == -1) {
			ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\kodiak-yellow.png");
			sawblade_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\sawbladeYellow.png");
			harpoon_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\harpoonYellow.png");
			tint_r = 255; tint_g = 230; tint_b = 70;
		} else {
			ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\kodiak-blue.png");
			sawblade_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\sawbladeBlue.png");
			harpoon_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\harpoonBlue.png");
			tint_r = 80; tint_g = 120; tint_b = 255;
		}
	} else {
		if (ally1 == -1) {
			ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\kodiak-green.png");
			sawblade_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\sawbladeGreen.png");
			harpoon_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\harpoonGreen.png");
			tint_r = 80; tint_g = 255; tint_b = 100;
		} else {
			ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\kodiak-teal.png");
			sawblade_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\sawbladeTeal.png");
			harpoon_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\harpoonTeal.png");
			tint_r = 70; tint_g = 220; tint_b = 255;
		}
	}

	ship_invincible_tex = r->LoadTexture("..\\Project1\\assets\\ships\\kodiak-white.png");

	cannon_tex = r->LoadTexture("..\\Project1\\assets\\cannon.png");

	bounce_sawblade_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\sawbladeOrange.png");

	roar_ring_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\roar_ring.png");
	SDL_SetTextureBlendMode(roar_ring_tex, SDL_BLENDMODE_BLEND);
	SDL_SetTextureColorMod(roar_ring_tex, tint_r, tint_g, tint_b);

	shield_tex = r->LoadTexture("..\\Project1\\assets\\shield.png");
	SDL_SetTextureAlphaMod(shield_tex, 100);
	bounce_tex = r->LoadTexture("..\\Project1\\assets\\bouncer.png");
	SDL_SetTextureAlphaMod(bounce_tex, 100);

	thrust_low_tex = r->LoadTexture("..\\Project1\\assets\\thrustlow.png");
	thrust_medium_tex = r->LoadTexture("..\\Project1\\assets\\thrustmed.png");
	thrust_high_tex = r->LoadTexture("..\\Project1\\assets\\thrusthigh.png");

	sawblade_throw_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\sawblade_throw.wav");
	sawblade_hit_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\sawblade_hit.wav");
	harpoon_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\harpoon.wav");
	roar_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\roar.wav");
	roar_charge_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\roar_charge.wav");
}

Kodiak::~Kodiak() {
	SDL_DestroyTexture(ship_tex);
	SDL_DestroyTexture(ship_invincible_tex);
	SDL_DestroyTexture(cannon_tex);
	SDL_DestroyTexture(sawblade_tex);
	SDL_DestroyTexture(bounce_sawblade_tex);
	SDL_DestroyTexture(harpoon_tex);
	SDL_DestroyTexture(roar_ring_tex);
	SDL_DestroyTexture(shield_tex);
	SDL_DestroyTexture(bounce_tex);
	SDL_DestroyTexture(thrust_low_tex);
	SDL_DestroyTexture(thrust_medium_tex);
	SDL_DestroyTexture(thrust_high_tex);

	Mix_FreeChunk(sawblade_throw_sfx);
	Mix_FreeChunk(sawblade_hit_sfx);
	Mix_FreeChunk(harpoon_sfx);
	Mix_FreeChunk(roar_sfx);
	Mix_FreeChunk(roar_charge_sfx);

	for (int i = 0; i < num_sawblades; i++) {
		delete sawblades[i];
	}
	for (int i = 0; i < num_sparks; i++) {
		delete sparks[i];
	}
}

void Kodiak::update() {
	gun_dir_x = desired_gun_dir_x;
	gun_dir_y = desired_gun_dir_y;
}

void Kodiak::die() {
	Mix_HaltChannel(roar_channel);

	if (r_state == roar_state::charging) {
		r_state = roar_state::idle;
		max_accel = normal_max_accel;
	}

	if (h_state == harpoon_state::tethered) {
		h_state = harpoon_state::retracting;
		harpoon_target = -1;
	}
}

bool Kodiak::is_enemy(Ship* s) {
	if (!s) return false;
	if (s->id == id) return false;
	if (s->id == ally1) return false;
	if (s->id == ally2) return false;
	if (s->lives == 0) return false;
	return true;
}

// ---------------------------------------------------------------- sparks (shared hit effect)

void Kodiak::add_sparks(int x, int y, int count) {
	for (int i = 0; i < count && num_sparks < 1000; i++) {
		sparks[num_sparks] = new Spark(x, y);
		num_sparks++;
	}
}

void Kodiak::update_sparks() {
	for (int i = 0; i < num_sparks; i++) {
		Spark* s = sparks[i];
		double angle = atan2(s->y_2 - s->y_1, s->x_2 - s->x_1);
		s->x_1 += (int)(cos(angle) * s->vel);
		s->y_1 += (int)(sin(angle) * s->vel);
		s->x_2 += (int)(cos(angle) * s->vel);
		s->y_2 += (int)(sin(angle) * s->vel);

		s->remaining_life--;
		if (s->remaining_life <= 0) {
			delete sparks[i];
			sparks[i] = sparks[num_sparks - 1];
			num_sparks--;
			i--;
		}
	}
}

void Kodiak::render_sparks() {
	if (!r->render_normal) return;
	r->SetRenderDrawColor(255, 220, 120, SDL_ALPHA_OPAQUE);
	for (int i = 0; i < num_sparks; i++) {
		Spark* s = sparks[i];
		r->render_line_w_end(s->x_1, s->y_1, s->x_2, s->y_2);
	}
}

// ---------------------------------------------------------------- RB: sawblades

void Kodiak::fire_1() {
	if (sawblade_cooldown > 0) {
		sawblade_cooldown--;
	}

	if (do_fire_1 && sawblade_cooldown <= 0 && num_sawblades < max_sawblades && stamina > 0) {
		double angle = atan2(gun_dir_y, gun_dir_x);
		int x = (int)(x_pos + gun_length * cos(angle));
		int y = (int)(y_pos + gun_length * sin(angle));

		// the blade inherits a little of the ship's momentum, like a real throw
		int x_v = (int)(sawblade_throw_speed * cos(angle)) + x_vel / 2;
		int y_v = (int)(sawblade_throw_speed * sin(angle)) + y_vel / 2;

		sawblades[num_sawblades] = new Sawblade(x, y, x_v, y_v);
		num_sawblades++;

		sawblade_cooldown = sawblade_delay;
		stamina -= 300;
		Mix_PlayChannel(-1, sawblade_throw_sfx, 0);
	}
}

void Kodiak::update_projectiles_1(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
	for (int j = 0; j < num_sawblades; j++) {
		Sawblade* b = sawblades[j];

		b->frames_alive++;
		b->spin += 24;
		if (b->spin >= 360) b->spin -= 360;

		for (int k = 0; k < 4; k++) {
			if (b->hit_cooldowns[k] > 0) b->hit_cooldowns[k]--;
		}

		if (!b->returning) {
			// the throw bleeds off energy, then the blade turns around (reach is roughly 400px)
			b->x_vel = (int)(b->x_vel * sawblade_drag);
			b->y_vel = (int)(b->y_vel * sawblade_drag);
			double speed = sqrt(pow(b->x_vel, 2) + pow(b->y_vel, 2));
			if (speed < 25000 || b->frames_alive > sawblade_outbound_frames) {
				b->returning = true;
			}
		} else {
			// home in on wherever the ship is now
			double dx = x_pos - b->x_pos;
			double dy = y_pos - b->y_pos;
			double dist = sqrt(dx * dx + dy * dy);
			if (dist < 1) dist = 1;

			// caught
			if (dist < radius + b->radius) {
				num_sawblades--;
				delete sawblades[j];
				sawblades[j] = sawblades[num_sawblades];
				j--;
				continue;
			}

			// safety net: if it somehow never makes it home, let it go
			if (b->frames_alive > 600) {
				num_sawblades--;
				delete sawblades[j];
				sawblades[j] = sawblades[num_sawblades];
				j--;
				continue;
			}

			b->x_vel += (int)(sawblade_return_accel * dx / dist);
			b->y_vel += (int)(sawblade_return_accel * dy / dist);

			double speed = sqrt(pow(b->x_vel, 2) + pow(b->y_vel, 2));
			if (speed > sawblade_max_speed) {
				b->x_vel = (int)(b->x_vel * sawblade_max_speed / speed);
				b->y_vel = (int)(b->y_vel * sawblade_max_speed / speed);
			}
		}

		b->x_pos += b->x_vel;
		b->y_pos += b->y_vel;

		// ricochet off the arena walls
		bool ricochet = false;
		if (b->x_pos < min_x + b->radius && b->x_vel < 0) {
			b->x_vel = -b->x_vel;
			b->x_pos = min_x + b->radius;
			ricochet = true;
		} else if (b->x_pos > max_x - b->radius && b->x_vel > 0) {
			b->x_vel = -b->x_vel;
			b->x_pos = max_x - b->radius;
			ricochet = true;
		}
		if (b->y_pos < min_y + b->radius && b->y_vel < 0) {
			b->y_vel = -b->y_vel;
			b->y_pos = min_y + b->radius;
			ricochet = true;
		} else if (b->y_pos > max_y - b->radius && b->y_vel > 0) {
			b->y_vel = -b->y_vel;
			b->y_pos = max_y - b->radius;
			ricochet = true;
		}
		if (ricochet) {
			add_sparks(b->x_pos, b->y_pos, 3);
		}

		// ricochet off asteroids
		for (int k = 0; k < num_asteroids; k++) {
			Asteroid* a = asteroids[k];
			double dx = b->x_pos - a->x_pos;
			double dy = b->y_pos - a->y_pos;
			double dist = sqrt(dx * dx + dy * dy);
			if (dist <= (b->radius + a->radius)) {
				if (dist < 1) { dx = 1; dy = 0; dist = 1; }
				double nx = dx / dist;
				double ny = dy / dist;
				double dot = b->x_vel * nx + b->y_vel * ny;
				if (dot < 0) {
					b->x_vel = (int)(b->x_vel - 2 * dot * nx);
					b->y_vel = (int)(b->y_vel - 2 * dot * ny);
					Mix_PlayChannel(-1, sawblade_hit_sfx, 0);
					add_sparks(b->x_pos, b->y_pos, 6);
				}
				// push it out of the rock
				b->x_pos = (int)(a->x_pos + nx * (a->radius + b->radius));
				b->y_pos = (int)(a->y_pos + ny * (a->radius + b->radius));
			}
		}

		// shred enemies; the blade passes through and keeps going
		for (int k = 0; k < 4; k++) {
			if (!is_enemy(ships[k])) continue;
			if (b->hit_cooldowns[k] > 0) continue;

			double dist = sqrt(pow(b->x_pos - ships[k]->x_pos, 2) + pow(b->y_pos - ships[k]->y_pos, 2));
			if (dist <= (ships[k]->radius + b->radius)) {
				bool hit = ships[k]->take_knockback(b->x_vel, b->y_vel, b->base_knockback, b->knockback_scaling, b->damage, haptics[k]);
				if (hit) {
					damage_done += b->damage;
					ships[k]->last_hit = id;
					add_sparks(b->x_pos, b->y_pos, 8);
					Mix_PlayChannel(-1, sawblade_hit_sfx, 0);
				}
				b->hit_cooldowns[k] = sawblade_rehit_delay;
			}
		}
	}
}

void Kodiak::render_projectiles_1() {
	SDL_Texture* tex = item_times[bullet_bounce] > 0 ? bounce_sawblade_tex : sawblade_tex;

	for (int j = 0; j < num_sawblades; j++) {
		Sawblade* b = sawblades[j];
		if (r->render_normal) {
			r->render_texture(tex, b->x_pos, b->y_pos, b->spin, 2);
		}
		if (r->render_debug) {
			r->render_texture_abs_size(r->hitbox_tex, b->x_pos, b->y_pos, 0, b->radius);
		}
	}
}

// ---------------------------------------------------------------- RT: harpoon

void Kodiak::fire_2() {
	if (harpoon_cooldown > 0) {
		harpoon_cooldown--;
	}

	// one harpoon per trigger pull
	if (!do_fire_2) {
		harpoon_click_used = false;
	}

	if (do_fire_2 && !harpoon_click_used && h_state == harpoon_state::ready && harpoon_cooldown <= 0 && stamina > 0) {
		harpoon_click_used = true;

		double angle = atan2(gun_dir_y, gun_dir_x);
		harpoon_x = (int)(x_pos + gun_length * cos(angle));
		harpoon_y = (int)(y_pos + gun_length * sin(angle));
		harpoon_x_vel = (int)(harpoon_speed * cos(angle));
		harpoon_y_vel = (int)(harpoon_speed * sin(angle));
		harpoon_angle = r->atan2_degrees(gun_dir_x, gun_dir_y);
		harpoon_dist_travelled = 0;
		harpoon_target = -1;
		harpoon_tether_frames = 0;

		h_state = harpoon_state::flying;
		stamina -= 450;
		Mix_PlayChannel(-1, harpoon_sfx, 0);
	}
}

void Kodiak::update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
	double gun_angle = atan2(gun_dir_y, gun_dir_x);
	int tip_x = (int)(x_pos + gun_length * cos(gun_angle));
	int tip_y = (int)(y_pos + gun_length * sin(gun_angle));

	if (h_state == harpoon_state::flying) {
		harpoon_x += harpoon_x_vel;
		harpoon_y += harpoon_y_vel;
		harpoon_dist_travelled += harpoon_speed;

		bool out_of_bounds = harpoon_x < min_x || harpoon_x > max_x || harpoon_y < min_y || harpoon_y > max_y;
		if (out_of_bounds || harpoon_dist_travelled > harpoon_max_range) {
			h_state = harpoon_state::retracting;
		}

		// clang off asteroids
		if (h_state == harpoon_state::flying) {
			for (int k = 0; k < num_asteroids; k++) {
				Asteroid* a = asteroids[k];
				double dist = sqrt(pow(harpoon_x - a->x_pos, 2) + pow(harpoon_y - a->y_pos, 2));
				if (dist <= (harpoon_radius + a->radius)) {
					add_sparks(harpoon_x, harpoon_y, 5);
					Mix_PlayChannel(-1, sawblade_hit_sfx, 0);
					h_state = harpoon_state::retracting;
					break;
				}
			}
		}

		// impale an enemy
		if (h_state == harpoon_state::flying) {
			for (int k = 0; k < 4; k++) {
				if (!is_enemy(ships[k])) continue;
				double dist = sqrt(pow(harpoon_x - ships[k]->x_pos, 2) + pow(harpoon_y - ships[k]->y_pos, 2));
				if (dist <= (ships[k]->radius + harpoon_radius)) {
					// the hit itself doesn't shove them; the reel-in does the work
					bool hit = ships[k]->take_knockback(harpoon_x_vel, harpoon_y_vel, 0, 0, harpoon_damage, haptics[k]);
					if (hit) {
						damage_done += harpoon_damage;
						ships[k]->last_hit = id;
						add_sparks(ships[k]->x_pos, ships[k]->y_pos, 10);

						h_state = harpoon_state::tethered;
						harpoon_target = k;
						harpoon_tether_frames = 0;
					} else {
						// shield or invincibility: bounce off
						h_state = harpoon_state::retracting;
					}
					break;
				}
			}
		}
	} else if (h_state == harpoon_state::tethered) {
		Ship* t = (harpoon_target >= 0) ? ships[harpoon_target] : NULL;

		bool broken = !t || t->lives == 0 || t->invincibility_cooldown > 0 || t->item_times[shield] > 0
			|| harpoon_tether_frames >= harpoon_tether_max || lives == 0;

		if (!broken) {
			harpoon_x = t->x_pos;
			harpoon_y = t->y_pos;

			double dx = x_pos - t->x_pos;
			double dy = y_pos - t->y_pos;
			double dist = sqrt(dx * dx + dy * dy);
			if (dist < 1) dist = 1;

			// they've arrived; let go
			if (dist < radius + t->radius + 12 * 10000) {
				broken = true;
			} else {
				// reel them in, and get dragged toward them a little in return
				t->x_vel += (int)(harpoon_pull * dx / dist);
				t->y_vel += (int)(harpoon_pull * dy / dist);

				x_vel -= (int)(harpoon_self_pull * dx / dist);
				y_vel -= (int)(harpoon_self_pull * dy / dist);

				harpoon_angle = r->atan2_degrees((int)-dx, (int)-dy);
				harpoon_tether_frames++;

				if (harpoon_tether_frames % 10 == 1) {
					SDL_HapticRumblePlay(haptics[harpoon_target], 0.25f, 80);
				}
			}
		}

		if (broken) {
			h_state = harpoon_state::retracting;
			harpoon_target = -1;
		}
	} else if (h_state == harpoon_state::retracting) {
		double dx = tip_x - harpoon_x;
		double dy = tip_y - harpoon_y;
		double dist = sqrt(dx * dx + dy * dy);

		if (dist <= harpoon_retract_speed) {
			h_state = harpoon_state::ready;
			harpoon_cooldown = harpoon_delay;
		} else {
			harpoon_x += (int)(harpoon_retract_speed * dx / dist);
			harpoon_y += (int)(harpoon_retract_speed * dy / dist);
			harpoon_angle = r->atan2_degrees((int)dx, (int)dy);
		}
	}
}

void Kodiak::render_projectiles_2() {
	if (h_state == harpoon_state::ready) return;

	double gun_angle = atan2(gun_dir_y, gun_dir_x);
	int tip_x = (int)(x_pos + gun_length * cos(gun_angle));
	int tip_y = (int)(y_pos + gun_length * sin(gun_angle));

	if (r->render_normal) {
		// chain: alternating light and dark links from the gun to the harpoon
		double dx = harpoon_x - tip_x;
		double dy = harpoon_y - tip_y;
		double dist = sqrt(dx * dx + dy * dy);
		int links = (int)(dist / (8 * 10000));
		if (links < 1) links = 1;
		for (int i = 0; i < links; i++) {
			int sx = (int)(tip_x + dx * i / links);
			int sy = (int)(tip_y + dy * i / links);
			int ex = (int)(tip_x + dx * (i + 1) / links);
			int ey = (int)(tip_y + dy * (i + 1) / links);
			if (i % 2 == 0) {
				r->SetRenderDrawColor(190, 195, 205, SDL_ALPHA_OPAQUE);
			} else {
				r->SetRenderDrawColor(80, 84, 95, SDL_ALPHA_OPAQUE);
			}
			r->render_line_w_end(sx, sy, ex, ey);
		}

		r->render_texture(harpoon_tex, harpoon_x, harpoon_y, harpoon_angle, 2);
	}
	if (r->render_debug) {
		r->render_texture_abs_size(r->hitbox_tex, harpoon_x, harpoon_y, 0, harpoon_radius);
	}
}

// ---------------------------------------------------------------- LB: roar

void Kodiak::fire_3() {
	if (roar_cooldown > 0) {
		roar_cooldown--;
	}

	if (r_state == roar_state::idle) {
		if (do_fire_3 && roar_cooldown <= 0 && stamina > 0) {
			r_state = roar_state::charging;
			roar_charge = 0;
			stamina -= 350;

			// dig in while winding up
			max_accel = charging_max_accel;

			Mix_PlayChannel(roar_channel, roar_charge_sfx, 0);
		}
	} else if (r_state == roar_state::charging) {
		roar_charge++;
		if (!do_fire_3 || roar_charge >= roar_max_charge) {
			release_roar();
		}
	}
}

void Kodiak::release_roar() {
	r_state = roar_state::blasting;
	roar_charge_at_release = roar_charge;
	roar_radius = radius;
	roar_max_radius = roar_min_max_radius + (int)((double)roar_extra_max_radius * roar_charge / roar_max_charge);
	roar_frames = 0;
	for (int i = 0; i < 4; i++) {
		roar_hit[i] = false;
	}

	// a bigger roar costs more breath
	stamina -= 6 * roar_charge;
	roar_cooldown = roar_delay;
	max_accel = normal_max_accel;

	Mix_HaltChannel(roar_channel);
	Mix_PlayChannel(-1, roar_sfx, 0);
}

void Kodiak::update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
	if (r_state == roar_state::blasting) {
		roar_radius += roar_expand_per_frame;
		roar_frames++;

		double power = (double)roar_charge_at_release / roar_max_charge;
		int damage = 2 + (int)(5 * power);
		int base_kb = 35 + (int)(65 * power);
		int scaling_kb = 45;

		for (int k = 0; k < 4; k++) {
			if (roar_hit[k]) continue;
			if (!is_enemy(ships[k])) continue;
			Ship* t = ships[k];

			double dx = t->x_pos - x_pos;
			double dy = t->y_pos - y_pos;
			double dist = sqrt(dx * dx + dy * dy);

			// the shockwave front has reached them
			if (dist - t->radius <= roar_radius) {
				if (dist < 1) { dx = 1; dy = 0; dist = 1; }
				int dir_x = (int)(1000 * dx / dist);
				int dir_y = (int)(1000 * dy / dist);
				if (dir_x == 0 && dir_y == 0) dir_x = 1;

				bool hit = t->take_knockback(dir_x, dir_y, base_kb, scaling_kb, damage, haptics[k]);
				if (hit) {
					damage_done += damage;
					t->last_hit = id;
					add_sparks(t->x_pos, t->y_pos, 6 + (int)(10 * power));
				}
				roar_hit[k] = true;
			}
		}

		// shove asteroids as the front passes over them
		for (int k = 0; k < num_asteroids; k++) {
			Asteroid* a = asteroids[k];
			double dx = a->x_pos - x_pos;
			double dy = a->y_pos - y_pos;
			double dist = sqrt(dx * dx + dy * dy);
			if (dist < 1) continue;
			double edge = dist - a->radius;
			if (edge <= roar_radius && edge > roar_radius - roar_expand_per_frame) {
				int shove = 15000 + (int)(25000 * power);
				a->x_vel += (int)(shove * dx / dist);
				a->y_vel += (int)(shove * dy / dist);
			}
		}

		if (roar_radius >= roar_max_radius) {
			r_state = roar_state::idle;
		}
	}

	update_sparks();
}

void Kodiak::render_projectiles_3() {
	if (r->render_normal) {
		if (r_state == roar_state::charging) {
			// a tight ring pulsing around the ship while it winds up
			double k = (double)roar_charge / roar_max_charge;
			int rr = radius + (int)(radius * 0.35 * k) + (int)(6 * 10000 * sin(roar_charge * 0.6));
			SDL_SetTextureAlphaMod(roar_ring_tex, (Uint8)(60 + 120 * k));
			r->render_texture_abs_size(roar_ring_tex, x_pos, y_pos, roar_charge * 7, rr);
		} else if (r_state == roar_state::blasting) {
			double progress = (double)(roar_radius - radius) / (double)(roar_max_radius - radius);
			if (progress < 0) progress = 0;
			if (progress > 1) progress = 1;

			SDL_SetTextureAlphaMod(roar_ring_tex, (Uint8)(230 * (1.0 - 0.85 * progress)));
			r->render_texture_abs_size(roar_ring_tex, x_pos, y_pos, roar_frames * 9, roar_radius);

			// a fainter echo ring trailing behind the front
			int echo = roar_radius - 40 * 10000;
			if (echo > radius) {
				SDL_SetTextureAlphaMod(roar_ring_tex, (Uint8)(110 * (1.0 - progress)));
				r->render_texture_abs_size(roar_ring_tex, x_pos, y_pos, -roar_frames * 6, echo);
			}
		}
	}
	if (r->render_debug && r_state == roar_state::blasting) {
		r->render_texture_abs_size(r->hitbox_tex, x_pos, y_pos, 0, roar_radius);
	}

	render_sparks();
}

int Kodiak::serialize(char* buf, int i) {
	return 0;
}

int Kodiak::deserialize(char*buf, int i) {
	return 0;
}

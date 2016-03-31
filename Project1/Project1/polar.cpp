#include <string>
#include <SDL.h>
#include "spaceship.h"
#include "spark.h"
#include "gravity-missile.h"
#include "polar.h"
#include "renderer.h"


#include "bullet.h"


Polar::Polar(int identifier, int x, int y) {
	id = identifier;

	x_pos = x;
	y_pos = y;

	respawn_x = x;
	respawn_y = y;

	stamina_max = 1600;
	stamina = stamina_max;
	stamina_per_frame = 4;

	max_accel = 7000;
	friction_limiter = 1200000;
	constant_friction = 12000000000;

	radius = 40 * 10000;
	weight = 60;

	if (id == 0) {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\polar-red.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\attacks\\bulletRed.png");
	} else if (id == 1) {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\polar-blue.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\attacks\\bulletBlue.png");
	} else if (id == 2) {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\polar-yellow.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\attacks\\bulletYellow.png");
	} else {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\polar-green.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\attacks\\bulletGreen.png");
	}

	ship_invincible_tex = LoadTexture("..\\Project1\\assets\\ships\\polar-white.png");

	cannon_tex = LoadTexture("..\\Project1\\assets\\cannon.png");

	missile_tex = LoadTexture("..\\Project1\\assets\\attacks\\missile.png");
	vortex_tex = LoadTexture("..\\Project1\\assets\\unused\\baddie.png");
	SDL_SetTextureAlphaMod(vortex_tex, 100);

}

void Polar::update() {
	double angle = atan2(gun_dir_y, gun_dir_x);
	double desired_angle = atan2(desired_gun_dir_y, desired_gun_dir_x);
	double new_angle = angle;
	
	double polar_gun_turn_speed = M_PI / 30;

	double angle_diff = desired_angle - angle;
	if (abs(angle_diff) > abs(angle - desired_angle + M_PI)) {
		angle_diff = angle - desired_angle + M_PI;
	}
	if (abs(angle_diff) > abs(angle - desired_angle - M_PI)) {
		angle_diff = angle - desired_angle - M_PI;
	}
	if (abs(angle_diff) > abs(desired_angle - angle - 2*M_PI)) {
		angle_diff = desired_angle - angle - 2*M_PI;
	}
	if (abs(angle_diff) > abs(desired_angle - angle + 2*M_PI)) {
		angle_diff = desired_angle - angle + 2*M_PI;
	}

	if (abs(angle_diff) <= polar_gun_turn_speed) {
		new_angle = desired_angle;
	}
	else if (angle_diff > 0) {
		new_angle += polar_gun_turn_speed;
	}
	else if (angle_diff < 0) {
		new_angle -= polar_gun_turn_speed;
	}

	gun_dir_x = 10000 * cos(new_angle);
	gun_dir_y = 10000 * sin(new_angle);
}

void Polar::fire_1() {
	// handle spread fire spawns
	if (spread_cooldown > 0) {
		spread_cooldown--;
	}
	if (do_fire_1 && stamina > 0 && spread_cooldown <= 0) {

		int MUZZLE_VEL = 40000;
		int spread = 5;
		bullet** new_bullets = spawn_bullets(gun_dir_x, gun_dir_y, x_pos, y_pos, MUZZLE_VEL, spread, 5, 10, 100);
		for (int i = 0; i < spread; i++) {
			bullets[num_bullets] = new_bullets[i];
			num_bullets++;
		}
		free(new_bullets);
		spread_cooldown += spread_delay;
		stamina -= 250;
	}
}

void Polar::update_projectiles_1(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) {
	for (int j = 0; j < num_bullets; j++) {
		struct bullet* bullet = bullets[j];

		bullet->x_vel += bullet->x_accel;
		bullet->y_vel += bullet->y_accel;

		bullet->x_pos += bullet->x_vel;
		bullet->y_pos += bullet->y_vel;

		// check for bullet going out of bounds
		if (bullet->x_pos < min_x || bullet->x_pos > max_x || bullet->y_pos < min_y || bullet->y_pos > max_y) {
			num_bullets--;
			free(bullets[j]);
			bullets[j] = bullets[num_bullets];
			j--;
			continue;
		}

		// check for collisions with enemies
		for (int k = 0; k < 4; k++) {
			if (!ships[k]) continue;
			if (ships[k]->id == id) continue;
			double dist = sqrt(pow(bullet->x_pos - ships[k]->x_pos, 2) + pow(bullet->y_pos - ships[k]->y_pos, 2));
			//std::cout << dist << std::endl;
			if (dist <= (ships[k]->radius + bullet->radius)) {

				ships[k]->take_knockback(bullet->x_vel, bullet->y_vel, bullet->base_knockback, bullet->knockback_scaling, bullet->damage, haptics[k]);

				// delete bullet
				num_bullets--;
				free(bullets[j]);
				bullets[j] = bullets[num_bullets];
				j--;
				break;
			}
		}
	}
}

void Polar::render_projectiles_1() {
	for (int j = 0; j < num_bullets; j++) {
		double angle = calculate_angle(bullets[j]->x_vel, bullets[j]->y_vel);
		render_texture(bullet_tex, bullets[j]->x_pos / 10000, bullets[j]->y_pos / 10000, angle, 1);
	}
}

void Polar::fire_2() {
	if (missile_cooldown > 0) {
		missile_cooldown--;
	}
	if (do_fire_2 && stamina > 0 && missile_cooldown <= 0) {
		double angle = atan2(gun_dir_y, gun_dir_x);
		int velocity = 100000;
		int x_vel = cos(angle) * velocity;
		int y_vel = sin(angle) * velocity;
		g_missiles[num_g_missiles] = new Gravity_Missile(x_pos, y_pos, x_vel, y_vel);
		num_g_missiles++;
		missile_cooldown += missile_delay;
		stamina -= 500;
	}
}

void Polar::update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) {
	for (int i = 0; i < num_g_missiles; i++) {
		Gravity_Missile* m = g_missiles[i];

		// expand missile radius
		if (m->exploded) {
			// rotate vortex
			m->vortex_angle += vortex_turn_rate;
			if (m->vortex_angle > 360) m->vortex_angle -= 360;

			m->radius += G_MISSILE_RADIUS_PER_FRAME;
			if (m->radius > G_MISSILE_MAX_RADIUS) {
				num_g_missiles--;
				free(g_missiles[i]);
				g_missiles[i] = g_missiles[num_g_missiles];
				i--;
				continue;
			}
		}

		m->x_pos += m->x_vel;
		m->y_pos += m->y_vel;

		// check for missile going out of bounds
		if (m->x_pos < min_x || m->x_pos > max_x || m->y_pos < min_y || m->y_pos > max_y) {
			num_g_missiles--;
			free(g_missiles[i]);
			g_missiles[i] = g_missiles[num_g_missiles];
			i--;
			continue;
		}

		// check for collisions with enemies
		for (int k = 0; k < 4; k++) {
			if (!ships[k]) continue;
			double dist = sqrt(pow(m->x_pos - ships[k]->x_pos, 2) + pow(m->y_pos - ships[k]->y_pos, 2));
			if (!m->exploded) {
				if (ships[k]->id == id) continue;
				if (dist <= G_MISSILE_ACTIVATION_RADIUS) {
					m->exploded = true;
					m->x_vel = 0;
					m->y_vel = 0;
				}
			} else {
				if (dist <= (ships[k]->radius + m->radius)) {

					ships[k]->take_knockback(ships[k]->x_pos - m->x_pos, ships[k]->y_pos - m->y_pos, m->base_knockback, m->knockback_scaling, m->damage, haptics[k]);
				}

				// do gravity effect
				double force = 20000000000000000.0 / pow(dist, 2);
				if (force > 70000) force = 70000;
				double angle = calculate_angle(m->x_pos - ships[k]->x_pos, m->y_pos - ships[k]->y_pos);
				ships[k]->x_vel += force * cos(angle);
				ships[k]->y_vel += force * sin(angle);
			}

			
		}
	}
}

void Polar::render_projectiles_2() {
	for (int j = 0; j < num_g_missiles; j++) {
		if (!g_missiles[j]->exploded) {
			double angle = calculate_angle(g_missiles[j]->x_vel, g_missiles[j]->y_vel);
			render_texture(missile_tex, g_missiles[j]->x_pos / 10000, g_missiles[j]->y_pos / 10000, angle, 3);
		} else {
			SDL_Rect rect;

			rect.w = g_missiles[j]->radius / 10000 * 2;
			rect.h = g_missiles[j]->radius / 10000 * 2;
			rect.x = g_missiles[j]->x_pos / 10000 - rect.w / 2;
			rect.y = g_missiles[j]->y_pos / 10000 - rect.h / 2;

			RenderCopyEx(vortex_tex, NULL, &rect, g_missiles[j]->vortex_angle, NULL, SDL_FLIP_NONE);
			RenderCopyEx(vortex_tex, NULL, &rect, -g_missiles[j]->vortex_angle, NULL, SDL_FLIP_NONE);
			RenderCopyEx(vortex_tex, NULL, &rect, g_missiles[j]->vortex_angle+180, NULL, SDL_FLIP_NONE);
			RenderCopyEx(vortex_tex, NULL, &rect, -g_missiles[j]->vortex_angle+180, NULL, SDL_FLIP_NONE);
		}
	}
}

void Polar::fire_3() {
	if (do_fire_3 && stamina > 0) {
		laser_active = true;
		stamina -= 10;
	} else {
		laser_active = false;
	}
}

void Polar::update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) {
	if (laser_active) {
		// update laser position
		double angle = atan2(gun_dir_y, gun_dir_x);
		laser_start_x = x_pos + GUN_LENGTH * cos(angle);
		laser_start_y = y_pos + GUN_LENGTH * sin(angle);
		laser_end_x = laser_start_x + gun_dir_x;
		laser_end_y = laser_start_y + gun_dir_y;

		for (int i = 0; i < 4; i++) {
			if (!ships[i]) continue;
			if (ships[i]->id == id) continue;
			Ship* target_ship = ships[i];
			double dist = get_dist(laser_start_x, laser_start_y, laser_end_x, laser_end_y, target_ship->x_pos, target_ship->y_pos);

			if (dist < target_ship->radius) {
				target_ship->take_knockback(laser_end_x - laser_start_x, laser_end_y - laser_start_y, 0, 10, 1, haptics[i]);
				sparks[num_sparks] = new Spark(target_ship->x_pos, target_ship->y_pos);
				num_sparks++;
			}
		}
		
	}

	// update sparks
	for (int i = 0; i < num_sparks; i++) {
		Spark* s = sparks[i];
		double angle = atan2(s->y_2 - s->y_1, s->x_2 - s->x_1);
		s->x_1 += cos(angle) * s->vel;
		s->y_1 += sin(angle) * s->vel;
		s->x_2 += cos(angle) * s->vel;
		s->y_2 += sin(angle) * s->vel;

		s->remaining_life--;
		if (s->remaining_life == 0) {
			free(sparks[i]);
			sparks[i] = sparks[num_sparks-1];
			num_sparks--;
			i--;
		}
	}
}

void Polar::render_projectiles_3() {
	if (laser_active) {
		render_line_thick(laser_start_x, laser_start_y, gun_dir_x, gun_dir_y);
	}

	for (int i = 0; i < num_sparks; i++) {
		Spark* s = sparks[i];
		// set color to yellow, cause sparks are yellow
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawLine(renderer, s->x_1 / 10000, s->y_1 / 10000, s->x_2 / 10000, s->y_2 / 10000);
	}
}

double Polar::get_dist(long x_1, long y_1, long x_2, long y_2, long x_0, long y_0) {
	double n_1_a = (x_2 - x_1);
	double n_1_b = (y_1 - y_0);
	double n_1 = n_1_a * n_1_b;
	double n_2_a = (x_1 - x_0);
	double n_2_b = (y_2 - y_1);
	double n_2 = n_2_a* n_2_b;
	double numerator = abs(n_1 - n_2);
	double denom = sqrt(pow(x_2 - x_1, 2) + pow(y_2 - y_1, 2));
	double dist = numerator / denom;

	return dist;
}
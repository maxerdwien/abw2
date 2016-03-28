#include <string>
#include <SDL.h>
#include "spaceship.h"
#include "black.h"
#include "renderer.h"

#include "bullet.h"

Black::Black(int identifier, int x, int y) {
	id = identifier;

	x_pos = x;
	y_pos = y;

	respawn_x = x;
	respawn_y = y;

	stamina_max = 700;
	stamina = stamina_max;
	stamina_per_frame = 8;

	max_accel = 7000;
	friction_limiter = 1200000;
	constant_friction = 12000000000;

	radius = 40;
	weight = 60;

	if (id == 0) {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\black-red.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\attacks\\bulletRed.png");
	} else if (id == 1) {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\black-blue.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\attacks\\bulletBlue.png");
	} else if (id == 2) {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\black-yellow.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\attacks\\bulletYellow.png");
	} else {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\black-green.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\attacks\\bulletGreen.png");
	}

	ship_invincible_tex = LoadTexture("..\\Project1\\assets\\ships\\black-white.png");

	cannon_tex = LoadTexture("..\\Project1\\assets\\cannon.png");

	flame_tex_1 = LoadTexture("..\\Project1\\assets\\attacks\\flame1.png");
	flame_tex_2 = LoadTexture("..\\Project1\\assets\\attacks\\flame2.png");
}

void Black::update() {
	return;
}

void Black::fire_1() {
	if (burst_cooldown_1 > 0) {
		burst_cooldown_1--;
	}
	if (burst_shot_current != 0 && burst_cooldown_2 > 0) {
		burst_cooldown_2--;
	}
	if (do_fire_1 && stamina > 0 && burst_cooldown_1 <= 0) {
		int MUZZLE_VEL = 90000;
		int spread = 1;
		bullet** new_bullets = spawn_bullets(gun_dir_x, gun_dir_y, x_pos, y_pos, MUZZLE_VEL, spread, 3, 150, 0);
		for (int i = 0; i < spread; i++) {
			bullets[num_bullets] = new_bullets[i];
			num_bullets++;
		}
		free(new_bullets);
		burst_cooldown_1 += burst_delay_1;
		burst_shot_current++;
		stamina -= 320;
	}
	if (burst_cooldown_2 <= 0) {
		int MUZZLE_VEL = 90000;
		int spread = 1;
		bullet** new_bullets = spawn_bullets(gun_dir_x, gun_dir_y, x_pos, y_pos, MUZZLE_VEL, spread, 3, 150, 0);
		for (int i = 0; i < spread; i++) {
			bullets[num_bullets] = new_bullets[i];
			num_bullets++;
		}
		free(new_bullets);
		burst_cooldown_2 += burst_delay_2;
		burst_shot_current++;
		if (burst_shot_current == burst_shot_number) {
			burst_shot_current = 0;
		}
	}
}

void Black::update_projectiles_1(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) {
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
			if (dist <= (ships[k]->radius + bullet->radius) * 10000) {

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

void Black::render_projectiles_1() {
	for (int j = 0; j < num_bullets; j++) {
		double angle = calculate_angle(bullets[j]->x_vel, bullets[j]->y_vel);
		render_texture(bullet_tex, bullets[j]->x_pos / 10000, bullets[j]->y_pos / 10000, angle, (double)bullets[j]->radius/5);
	}
}

void Black::fire_2() {
	if (do_fire_2) {
		// charge rate doubles after the first half second of charging
		if (charge_shot_charge >= 30) {
			charge_shot_charge += 2;
		} else {
			charge_shot_charge++;
		}
		stamina -= 10;
	}
	if ((!do_fire_2 || stamina <= 0) && charge_shot_charge > 10) {
		int MUZZLE_VEL = 100000;
		int spread = 1;
		bullet** new_bullets = spawn_bullets(gun_dir_x, gun_dir_y, x_pos, y_pos, MUZZLE_VEL, spread, charge_shot_charge/4, charge_shot_charge, charge_shot_charge/2);
		for (int i = 0; i < spread; i++) {
			new_bullets[i]->radius = charge_shot_charge / 4;
			bullets[num_bullets] = new_bullets[i];
			num_bullets++;
		}
		free(new_bullets);
		charge_shot_charge = 0;
		if (stamina <= 0) {
			do_fire_2 = false;
		}
	}
}

void Black::update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) {
	// do nothing, because update_projectiles_1 does all the work
}

void Black::render_projectiles_2() {
	
}

void Black::fire_3() {
	if (do_fire_3 && stamina > 0) {
		flame_active = true;
		stamina -= 10;
	} else {
		flame_active = false;
	}
}

void Black::update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) {
	if (!flame_active) return;
	double angle = atan2(gun_dir_y, gun_dir_x);
	for (int i = 0; i < num_flame_hitboxes; i++) {
		int hb_x = x_pos + flame_dists[i] * cos(angle);
		int hb_y = y_pos + flame_dists[i] * sin(angle);

		for (int j = 0; j < 4; j++) {
			if (!ships[j]) continue;
			if (ships[j]->id == id) continue;
			Ship* target_ship = ships[j];
			double dist = sqrt(pow(hb_x - target_ship->x_pos, 2) + pow(hb_y - target_ship->y_pos, 2));
			if (dist <= (10000 * target_ship->radius + flame_radii[i])) {
				target_ship->take_knockback(target_ship->x_pos - hb_x, target_ship->y_pos - hb_y, 0, 6, 1, haptics[j]);
			}
		}
	}
	 
}

void Black::render_projectiles_3() {
	if (flame_active) {
		if (flame_switch_cooldown > 0) {
			flame_switch_cooldown--;
		}
		if (flame_switch_cooldown == 0) {
			current_flame = (current_flame + 1) % 2;
			flame_switch_cooldown = flame_switch_delay;
		}
		SDL_Rect rect;

		SDL_QueryTexture(flame_tex_1, NULL, NULL, &rect.w, &rect.h);
		rect.w *= 2;
		rect.h *= 2;
		double angle = atan2(gun_dir_y, gun_dir_x);
		rect.x = x_pos/10000 - rect.w / 2 + (12 + rect.h / 2)*cos(angle);
		rect.y = y_pos/10000 - rect.h / 2 + (12 + rect.h / 2)*sin(angle);

		if (current_flame == 0) {
			RenderCopyEx(flame_tex_1, NULL, &rect, calculate_angle(gun_dir_x, gun_dir_y), NULL, SDL_FLIP_NONE);
		} else {
			// works since the textures are the same size
			RenderCopyEx(flame_tex_2, NULL, &rect, calculate_angle(gun_dir_x, gun_dir_y), NULL, SDL_FLIP_NONE);
		}
	}
}
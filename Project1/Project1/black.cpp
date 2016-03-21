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
	friction_limiter = 1600000;
	constant_friction = 200000000;

	radius = 40;
	weight = 60;

	if (id == 0) {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\black-red.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\bulletRed.png");
	} else if (id == 1) {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\black-blue.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\bulletBlue.png");
	} else if (id == 2) {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\black-yellow.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\bulletYellow.png");
	} else {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\black-green.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\bulletGreen.png");
	}

	ship_invincible_tex = LoadTexture("..\\Project1\\assets\\ships\\black-white.png");

	cannon_tex = LoadTexture("..\\Project1\\assets\\cannon.png");
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

void Black::update_projectiles_1(int min_x, int max_x, int min_y, int max_y, int num_players, Ship* ships[], SDL_Haptic* haptics[]) {
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
		for (int k = 0; k < num_players; k++) {
			if (ships[k]->id == id) continue;
			double dist = sqrt(pow(bullet->x_pos - ships[k]->x_pos, 2) + pow(bullet->y_pos - ships[k]->y_pos, 2));
			//std::cout << dist << std::endl;
			if (dist <= (ships[k]->radius + bullet->radius) * 10000) {
				if (ships[k]->invincibility_cooldown == 0) {
					// knockback
					int total_knockback = (int)(100 * (bullet->base_knockback + (ships[k]->percent / 100.0)*bullet->knockback_scaling) / ships[k]->weight);
					
					ships[k]->x_vel += (int)(1000.0*total_knockback*bullet->x_vel / sqrt(pow(bullet->x_vel, 2) + pow(bullet->y_vel, 2)));
					ships[k]->y_vel += (int)(1000.0*total_knockback*bullet->y_vel / sqrt(pow(bullet->x_vel, 2) + pow(bullet->y_vel, 2)));
					//printf("bullet x_vel:%d\tbullet y_vel:%d\tship x_vel:%d\tship y_vel:%d\n", bullet->x_vel, bullet->y_vel, ships[k]->x_vel, ships[k]->y_vel);

					// damage
					ships[k]->percent += bullet->damage;
					if (ships[k]->percent > SPACESHIP_MAX_PERCENT) {
						ships[k]->percent = SPACESHIP_MAX_PERCENT;
					}

					// haptic
					float haptic_amount = 0.03f + total_knockback / 100.0f;
					if (haptic_amount > 1) {
						haptic_amount = 1;
					}
					SDL_HapticRumblePlay(haptics[k], haptic_amount, 160);
				}
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
	if ((!do_fire_2 || stamina <= 0) && charge_shot_charge > 0) {
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

void Black::update_projectiles_2(int min_x, int max_x, int min_y, int max_y, int num_players, Ship* ships[], SDL_Haptic* haptics[]) {
	// do nothing, because update_projectiles_1 does all the work
}

void Black::render_projectiles_2() {
	
}

void Black::fire_3() {

}

void Black::update_projectiles_3(int min_x, int max_x, int min_y, int max_y, int num_players, Ship* ships[], SDL_Haptic* haptics[]) {

}

void Black::render_projectiles_3() {

}
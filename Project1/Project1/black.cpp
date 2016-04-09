#include <string>
#include <SDL.h>
#include <SDL_mixer.h>

#include "asteroid.h"

#include "spaceship.h"
#include "black.h"
#include "renderer.h"

#include "item.h"

#include "bullet.h"

Black::Black(int identifier, int x, int y, Renderer* rend) {
	id = identifier;

	x_pos = x;
	y_pos = y;

	respawn_x = x;
	respawn_y = y;

	stamina_max = 700;
	stamina = stamina_max;
	stamina_per_frame = 8;

	max_accel = 10000;

	radius = 40 * 10000;
	normal_radius = radius;
	weight = 60;

	r = rend;

	charging_channel = 24 + 2*id;
	flamethrower_channel = 25 + 2 * id;

	if (id == 0) {
		ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\black-red.png");
		bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletRed.png");
	} else if (id == 1) {
		ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\black-blue.png");
		bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletBlue.png");
	} else if (id == 2) {
		ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\black-yellow.png");
		bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletYellow.png");
	} else {
		ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\black-green.png");
		bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletGreen.png");
	}

	ship_invincible_tex = r->LoadTexture("..\\Project1\\assets\\ships\\black-white.png");

	cannon_tex = r->LoadTexture("..\\Project1\\assets\\cannon.png");

	flame_tex_1 = r->LoadTexture("..\\Project1\\assets\\attacks\\flame1.png");
	flame_tex_2 = r->LoadTexture("..\\Project1\\assets\\attacks\\flame2.png");

	hitbox_tex = r->LoadTexture("..\\Project1\\assets\\sun.png");
	shield_tex = r->LoadTexture("..\\Project1\\assets\\shield.png");
	SDL_SetTextureAlphaMod(shield_tex, 100);
	bounce_tex = r->LoadTexture("..\\Project1\\assets\\shield.png");
	SDL_SetTextureAlphaMod(bounce_tex, 100);

	bullet_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\bullet.wav");
	flamethrower_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\flamethrower.wav");
	charging_shot_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\charging.wav");
}

void Black::update() {
	gun_dir_x = desired_gun_dir_x;
	gun_dir_y = desired_gun_dir_y;
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
		Mix_PlayChannel(-1, bullet_sfx, 0);
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
		Mix_PlayChannel(-1, bullet_sfx, 0);
		if (burst_shot_current == burst_shot_number) {
			burst_shot_current = 0;
		}
	}
}

void Black::update_projectiles_1(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
	for (int j = 0; j < num_bullets; j++) {
		struct bullet* bullet = bullets[j];

		bullet->x_vel += bullet->x_accel;
		bullet->y_vel += bullet->y_accel;

		bullet->x_pos += bullet->x_vel;
		bullet->y_pos += bullet->y_vel;

		// check for bullet going out of bounds
		if (bullet->x_pos < min_x || bullet->x_pos > max_x || bullet->y_pos < min_y || bullet->y_pos > max_y) {
			if (item_times[bullet_bounce] > 0) {
				if (bullet->x_pos < min_x && bullet->x_vel < 0) {
					bullet->x_vel *= -1;
				} else if (bullet->x_pos > max_x && bullet->x_vel > 0) {
					bullet->x_vel *= -1;
				} else if (bullet->y_pos < min_y && bullet->y_vel < 0) {
					bullet->y_vel *= -1;
				} else if (bullet->y_pos > max_y && bullet->y_vel > 0) {
					bullet->y_vel *= -1;
				}
			} else {
				num_bullets--;
				free(bullets[j]);
				bullets[j] = bullets[num_bullets];
				j--;
				continue;
			}
			
		}

		// check for collisions with asteroids
		for (int k = 0; k < num_asteroids; k++) {
			Asteroid* a = asteroids[k];
			double dist = sqrt(pow(a->x_pos - bullet->x_pos, 2) + pow(a->y_pos - bullet->y_pos, 2));
			if (dist <= (bullet->radius + a->radius)) {
				// todo: make bullets bounce if they have the powerup
				num_bullets--;
				free(bullets[j]);
				bullets[j] = bullets[num_bullets];
				j--;
				continue;
			}
		}

		// check for collisions with enemies
		for (int k = 0; k < 4; k++) {
			if (!ships[k]) continue;
			if (ships[k]->id == id) continue;
			if (ships[k]->lives == 0) continue;
			double dist = sqrt(pow(bullet->x_pos - ships[k]->x_pos, 2) + pow(bullet->y_pos - ships[k]->y_pos, 2));
			//std::cout << dist << std::endl;
			if (dist <= (ships[k]->radius + bullet->radius)) {

				ships[k]->take_knockback(bullet->x_vel, bullet->y_vel, bullet->base_knockback, bullet->knockback_scaling, bullet->damage, haptics[k]);
				damage_done += bullet->damage;
				ships[k]->last_hit = id;

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
		double angle = r->calculate_angle(bullets[j]->x_vel, bullets[j]->y_vel);
		r->render_texture(bullet_tex, bullets[j]->x_pos, bullets[j]->y_pos, angle, ((double)bullets[j]->radius/10000.0)/5);
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
		if (!Mix_Playing(charging_channel)) {
			Mix_PlayChannel(charging_channel, charging_shot_sfx, 0);
		}
	} else {
		Mix_HaltChannel(charging_channel);
	}
	if ((!do_fire_2 || stamina <= 0) && charge_shot_charge > 10) {
		int MUZZLE_VEL = 100000;
		int spread = 1;
		bullet** new_bullets = spawn_bullets(gun_dir_x, gun_dir_y, x_pos, y_pos, MUZZLE_VEL, spread, charge_shot_charge/4, charge_shot_charge, charge_shot_charge/2);
		for (int i = 0; i < spread; i++) {
			new_bullets[i]->radius = 10000 * charge_shot_charge / 4;
			bullets[num_bullets] = new_bullets[i];
			num_bullets++;
		}
		free(new_bullets);
		charge_shot_charge = 0;
		Mix_PlayChannel(-1, bullet_sfx, 0);
		if (stamina <= 0) {
			do_fire_2 = false;
		}
	}
}

void Black::update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
	// do nothing, because update_projectiles_1 does all the work
}

void Black::render_projectiles_2() {
	// same deal here
}

void Black::fire_3() {
	if (do_fire_3 && stamina > 0) {
		flame_active = true;
		stamina -= 10;
	} else {
		flame_active = false;
		do_fire_3 = false;
	}
}

void Black::update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
	if (!flame_active) return;
	double angle = atan2(gun_dir_y, gun_dir_x);
	for (int i = 0; i < num_flame_hitboxes; i++) {
		int hb_x = x_pos + flame_dists[i] * cos(angle);
		int hb_y = y_pos + flame_dists[i] * sin(angle);

		for (int j = 0; j < 4; j++) {
			if (!ships[j]) continue;
			if (ships[j]->id == id) continue;
			if (ships[j]->lives == 0) continue;
			Ship* target_ship = ships[j];
			double dist = sqrt(pow(hb_x - target_ship->x_pos, 2) + pow(hb_y - target_ship->y_pos, 2));
			if (dist <= (target_ship->radius + flame_radii[i])) {
				// direction is a combination of relation to hitbox and ship
				int x_dir = (target_ship->x_pos - hb_x) + (target_ship->x_pos - x_pos)/2;
				int y_dir = (target_ship->y_pos - hb_y) + (target_ship->y_pos - y_pos)/2;
				target_ship->take_knockback(x_dir, y_dir, 0, 4, 1, haptics[j]);
				damage_done += 1;
				target_ship->last_hit = id;
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

		double angle = atan2(gun_dir_y, gun_dir_x);

		// render hitboxes
		bool render_hitboxes = false;
		if (render_hitboxes) {
			for (int i = 0; i < num_flame_hitboxes; i++) {
				int hb_x = x_pos + flame_dists[i] * cos(angle);
				int hb_y = y_pos + flame_dists[i] * sin(angle);
				r->render_texture_abs_size(hitbox_tex, hb_x, hb_y, 0, flame_radii[i]);
			}
		}


		if (current_flame == 0) {
			r->render_texture_edge_spin(flame_tex_1, x_pos + GUN_LENGTH*cos(angle), y_pos + GUN_LENGTH*sin(angle), angle * 180 / M_PI + 90, 4);
		} else {
			// works since the textures are the same size
			r->render_texture_edge_spin(flame_tex_2, x_pos + GUN_LENGTH*cos(angle), y_pos + GUN_LENGTH*sin(angle), angle * 180 / M_PI + 90, 4);
		}

		if (!Mix_Playing(flamethrower_channel)) {
			Mix_PlayChannel(flamethrower_channel, flamethrower_sfx, 0);
		}
	} else {
		Mix_HaltChannel(flamethrower_channel);
	}
}
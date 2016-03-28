#include <string>
#include <SDL.h>
#include "spaceship.h"
#include "polar.h"
#include "renderer.h"

#include "bullet.h"

Polar::Polar(int identifier, int x, int y) {
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

}

void Polar::update() {
	return;
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

void Polar::render_projectiles_1() {
	for (int j = 0; j < num_bullets; j++) {
		double angle = calculate_angle(bullets[j]->x_vel, bullets[j]->y_vel);
		render_texture(bullet_tex, bullets[j]->x_pos / 10000, bullets[j]->y_pos / 10000, angle, (double)bullets[j]->radius / 5);
	}
}

void Polar::fire_2() {
}

void Polar::update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) {
	// do nothing, because update_projectiles_1 does all the work
}

void Polar::render_projectiles_2() {

}

void Polar::fire_3() {
}

void Polar::update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) {
}

void Polar::render_projectiles_3() {
}
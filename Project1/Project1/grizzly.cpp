#include <string>
#include <SDL.h>
#include "spaceship.h"
#include "grizzly.h"
#include "renderer.h"


#include "bullet.h"
#include "missile.h"

Grizzly::Grizzly(int identifier, int x, int y) {
	id = identifier;

	x_pos = x;
	y_pos = y;

	respawn_x = x;
	respawn_y = y;

	stamina_max = 1000;
	stamina = stamina_max;
	stamina_per_frame = 5;

	max_accel = 7000;
	friction_limiter = 9000000;
	constant_friction = 25000000000;

	radius = 40;
	weight = 100;

	if (id == 0) {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\grizzly-red.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\bulletRed.png");
		mine_tex = LoadTexture("..\\Project1\\assets\\mineRed.png");
	} else if (id == 1) {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\grizzly-blue.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\bulletBlue.png");
		mine_tex = LoadTexture("..\\Project1\\assets\\mineBlue.png");
	} else if (id == 2) {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\grizzly-yellow.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\bulletYellow.png");
		mine_tex = LoadTexture("..\\Project1\\assets\\mineYellow.png");
	} else {
		ship_tex = LoadTexture("..\\Project1\\assets\\ships\\grizzly-green.png");
		bullet_tex = LoadTexture("..\\Project1\\assets\\bulletGreen.png");
		mine_tex = LoadTexture("..\\Project1\\assets\\mineGreen.png");
	}

	ship_invincible_tex = LoadTexture("..\\Project1\\assets\\ships\\grizzly-white.png");

	cannon_tex = LoadTexture("..\\Project1\\assets\\cannon.png");
	
	missile_tex = LoadTexture("..\\Project1\\assets\\missile.png");
	explosion_tex = LoadTexture("..\\Project1\\assets\\explosion.png");
}

void Grizzly::update() {
	return;
}

void Grizzly::fire_1() {
	if (cannon_cooldown > 0) {
		cannon_cooldown--;
	}
	if (do_fire_1 && stamina > 0 && cannon_cooldown <= 0) {
		int MUZZLE_VEL = 60000;
		int spread = 1;
		bullet** new_bullets = spawn_bullets(gun_dir_x, gun_dir_y, x_pos, y_pos, MUZZLE_VEL, spread, 5, 10, 400);
		for (int i = 0; i < spread; i++) {
			bullets[num_bullets] = new_bullets[i];
			num_bullets++;
		}
		free(new_bullets);
		cannon_cooldown += cannon_delay;
		stamina -= 100;
	}
}

void Grizzly::update_projectiles_1(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) {
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

void Grizzly::render_projectiles_1() {
	for (int j = 0; j < num_bullets; j++) {
		double angle = calculate_angle(bullets[j]->x_vel, bullets[j]->y_vel);
		render_texture(bullet_tex, bullets[j]->x_pos / 10000, bullets[j]->y_pos / 10000, angle, 1);
	}
}

void Grizzly::fire_2() {
	if (missile_cooldown > 0) {
		missile_cooldown--;
	}
	if (do_fire_2 && stamina > 0 && missile_cooldown <= 0) {
		int MUZZLE_VEL = 70000;
		int spread = 1;
		missile** new_missiles = spawn_missiles(gun_dir_x, gun_dir_y, x_pos, y_pos, MUZZLE_VEL, spread, 25, 200, 300);
		for (int i = 0; i < spread; i++) {
			missiles[num_missiles] = new_missiles[i];
			num_missiles++;
		}
		free(new_missiles);
		missile_cooldown += missile_delay;
		stamina -= 400;
	}
}

void Grizzly::update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) {
	for (int j = 0; j < num_missiles; j++) {
		struct missile* missile = missiles[j];

		if (!missile->exploded) {
			// find the closest enemy player. this is the one to lock on to.
			double min_dist = 99999999;
			int target_player = -1;
			for (int k = 0; k < 4; k++) {
				if (!ships[k]) continue;
				if (ships[k]->id == id) continue;
				double dist = sqrt(pow(missile->x_pos - ships[k]->x_pos, 2) + pow(missile->y_pos - ships[k]->y_pos, 2));
				if (dist < min_dist) {
					min_dist = dist;
					target_player = k;
				}
			}

			// accelerate towards target player
			double delta_x = ships[target_player]->x_pos - missile->x_pos;
			double delta_y = ships[target_player]->y_pos - missile->y_pos;
			double delta_mag = sqrt(pow(delta_x, 2) + pow(delta_y, 2));

			missile->x_accel = MISSILE_ACCEL * delta_x / delta_mag;
			missile->y_accel = MISSILE_ACCEL * delta_y / delta_mag;

			missile->x_vel += missile->x_accel;
			missile->y_vel += missile->y_accel;

			missile->x_pos += missile->x_vel;
			missile->y_pos += missile->y_vel;
		}

		// check for missile going out of bounds
		if (missile->x_pos < min_x || missile->x_pos > max_x || missile->y_pos < min_y || missile->y_pos > max_y) {
			num_missiles--;
			free(missiles[j]);
			missiles[j] = missiles[num_missiles];
			j--;
			continue;
		}

		// expand missile radius
		if (missile->exploded) {
			missile->radius += MISSILE_RADIUS_PER_FRAME;
			if (missile->radius > MISSILE_MAX_RADIUS) {
				num_missiles--;
				free(missiles[j]);
				missiles[j] = missiles[num_missiles];
				j--;
				continue;
			}
		}

		// check for collisions with enemies
		for (int k = 0; k < 4; k++) {
			if (!ships[k]) continue;
			double dist = sqrt(pow(missile->x_pos - ships[k]->x_pos, 2) + pow(missile->y_pos - ships[k]->y_pos, 2));
			if (!missile->exploded) {
				if (ships[k]->id == id) continue;
				if (dist <= MISSILE_ACTIVATION_RADIUS * 10000) {
					missile->exploded = true;
					missile->x_vel = 0;
					missile->y_vel = 0;
				}
			} else {
				if (dist <= (ships[k]->radius + missile->radius) * 10000) {
					if (missile->players_hit[k]) continue;
					missile->players_hit[k] = true;

					ships[k]->take_knockback(ships[k]->x_pos - missile->x_pos, ships[k]->y_pos - missile->y_pos, missile->base_knockback, missile->knockback_scaling, missile->damage, haptics[k]);
				}
			}
		}
	}
}

void Grizzly::render_projectiles_2() {
	for (int j = 0; j < num_missiles; j++) {
		if (!missiles[j]->exploded) {
			double angle = calculate_angle(missiles[j]->x_vel, missiles[j]->y_vel);
			render_texture(missile_tex, missiles[j]->x_pos / 10000, missiles[j]->y_pos / 10000, angle, 1.8);
		} else {
			SDL_Rect rect;

			rect.w = missiles[j]->radius * 2;
			rect.h = missiles[j]->radius * 2;
			rect.x = missiles[j]->x_pos / 10000 - rect.w / 2;
			rect.y = missiles[j]->y_pos / 10000 - rect.h / 2;

			RenderCopyEx(explosion_tex, NULL, &rect, 0, NULL, SDL_FLIP_NONE);
		}
	}
}

void Grizzly::fire_3() {
	if (mine_cooldown > 0) {
		mine_cooldown--;
	}
	if (do_fire_3 && stamina > 0 && mine_cooldown <= 0) {
		// todo: make this zero
		int MUZZLE_VEL = 1000;
		int spread = 1;
		missile** new_missiles = spawn_missiles(gun_dir_x, gun_dir_y, x_pos, y_pos, MUZZLE_VEL, spread, 25, 200, 300);
		for (int i = 0; i < spread; i++) {
			mines[num_mines] = new_missiles[i];
			num_mines++;
		}
		free(new_missiles);
		mine_cooldown += mine_delay;
		stamina -= 300;
	}
}

void Grizzly::update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) {
	for (int j = 0; j < num_mines; j++) {
		struct missile* mine = mines[j];

		if (!mine->exploded) {
			// find the closest enemy player. this is the one to lock on to.
			double min_dist = 10000 * 200;
			int target_player = -1;
			for (int k = 0; k < 4; k++) {
				if (!ships[k]) continue;
				if (ships[k]->id == id) continue;
				double dist = sqrt(pow(mine->x_pos - ships[k]->x_pos, 2) + pow(mine->y_pos - ships[k]->y_pos, 2));
				if (dist < min_dist) {
					min_dist = dist;
					target_player = k;
				}
			}
			if (target_player != -1) {

				// accelerate towards target player
				double delta_x = ships[target_player]->x_pos - mine->x_pos;
				double delta_y = ships[target_player]->y_pos - mine->y_pos;
				double delta_mag = sqrt(pow(delta_x, 2) + pow(delta_y, 2));

				mine->x_accel = MISSILE_ACCEL * delta_x / delta_mag;
				mine->y_accel = MISSILE_ACCEL * delta_y / delta_mag;
			} else {
				mine->x_accel = 0;
				mine->y_accel = 0;
			}

			mine->x_vel += mine->x_accel;
			mine->y_vel += mine->y_accel;

			mine->x_pos += mine->x_vel;
			mine->y_pos += mine->y_vel;
		}

		// check for missile going out of bounds
		if (mine->x_pos < min_x || mine->x_pos > max_x || mine->y_pos < min_y || mine->y_pos > max_y) {
			num_mines--;
			free(mines[j]);
			mines[j] = mines[num_mines];
			j--;
			continue;
		}

		// expand missile radius
		if (mine->exploded) {
			mine->radius += MISSILE_RADIUS_PER_FRAME;
			if (mine->radius > MISSILE_MAX_RADIUS) {
				num_mines--;
				free(mines[j]);
				mines[j] = mines[num_mines];
				j--;
				continue;
			}
		}

		// check for collisions with enemies
		for (int k = 0; k < 4; k++) {
			if (!ships[k]) continue;

			double dist = sqrt(pow(mine->x_pos - ships[k]->x_pos, 2) + pow(mine->y_pos - ships[k]->y_pos, 2));
			if (!mine->exploded) {
				if (ships[k]->id == id) continue;
				if (dist <= MISSILE_ACTIVATION_RADIUS * 10000) {
					mine->exploded = true;
					mine->x_vel = 0;
					mine->y_vel = 0;
				}
			} else {
				if (dist <= (ships[k]->radius + mine->radius) * 10000) {
					if (mine->players_hit[k]) continue;
					mine->players_hit[k] = true;

					ships[k]->take_knockback(ships[k]->x_pos - mine->x_pos, ships[k]->y_pos - mine->y_pos, mine->base_knockback, mine->knockback_scaling, mine->damage, haptics[k]);
				}
			}
		}
	}
}

void Grizzly::render_projectiles_3() {
	for (int j = 0; j < num_mines; j++) {
		if (!mines[j]->exploded) {
			double angle = calculate_angle(mines[j]->x_vel, mines[j]->y_vel);
			render_texture(mine_tex, mines[j]->x_pos / 10000, mines[j]->y_pos / 10000, angle, 1.8);
		} else {
			SDL_Rect rect;

			rect.w = mines[j]->radius * 2;
			rect.h = mines[j]->radius * 2;
			rect.x = mines[j]->x_pos / 10000 - rect.w / 2;
			rect.y = mines[j]->y_pos / 10000 - rect.h / 2;

			RenderCopyEx(explosion_tex, NULL, &rect, 0, NULL, SDL_FLIP_NONE);
		}
	}
}
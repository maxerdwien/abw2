#include <string>
#include <SDL.h>
#include <SDL_image.h>

#include "spaceship.h"
#include "bullet.h"
#include "missile.h"


#include "renderer.h"

Grizzly::Grizzly(int identifier, int x, int y) {
	id = identifier;

	x_pos = x;
	y_pos = y;

	stamina_max = 1000;
	stamina = stamina_max;
	radius = 40;
	weight = 1;

	if (id == 0) {
		bullet_tex = LoadTexture("..\\Project1\\assets\\bulletBlue.png");
	} else if (id == 1) {
		bullet_tex = LoadTexture("..\\Project1\\assets\\bulletRed.png");
	} else if (id == 2) {
		bullet_tex = LoadTexture("..\\Project1\\assets\\bulletYellow.png");
	} else {
		bullet_tex = LoadTexture("..\\Project1\\assets\\bulletGreen.png");
	}

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

void Grizzly::update_projectiles_1(int min_x, int max_x, int min_y, int max_y, int num_players, Ship* ships[]) {
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
					int total_knockback = (int)((bullet->base_knockback + (ships[k]->percent / 100.0)*bullet->knockback_scaling) / ships[k]->weight);
					ships[k]->x_vel += (int)(1000.0*total_knockback*bullet->x_vel / sqrt(pow(bullet->x_vel, 2) + pow(bullet->y_vel, 2)));
					ships[k]->y_vel += (int)(1000.0*total_knockback*bullet->y_vel / sqrt(pow(bullet->x_vel, 2) + pow(bullet->y_vel, 2)));

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
					//SDL_HapticRumblePlay(haptics[k], haptic_amount, 160);
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

void Grizzly::update_projectiles_2(int min_x, int max_x, int min_y, int max_y, int num_players, Ship* ships[]) {
	for (int j = 0; j < num_missiles; j++) {
		struct missile* missile = missiles[j];

		if (!missile->exploded) {
			// find the closest enemy player. this is the one to lock on to.
			double min_dist = 99999999;
			int target_player = -1;
			for (int k = 0; k < num_players; k++) {
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
		for (int k = 0; k < num_players; k++) {
			//if (i == k) continue;

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
					if (ships[k]->invincibility_cooldown == 0) {

						// knockback
						int total_knockback = (int)((missile->base_knockback + (ships[k]->percent / 100.0)*missile->knockback_scaling) / ships[k]->weight);
						ships[k]->x_vel += (int)(1000.0*total_knockback*(ships[k]->x_pos - missile->x_pos) / sqrt(pow(missile->x_pos - ships[k]->x_pos, 2) + pow(missile->y_pos - ships[k]->y_pos, 2)));
						ships[k]->y_vel += (int)(1000.0*total_knockback*(ships[k]->y_pos - missile->y_pos) / sqrt(pow(missile->x_pos - ships[k]->x_pos, 2) + pow(missile->y_pos - ships[k]->y_pos, 2)));

						// damage
						ships[k]->percent += missile->damage;
						if (ships[k]->percent > SPACESHIP_MAX_PERCENT) {
							ships[k]->percent = SPACESHIP_MAX_PERCENT;
						}

						// haptic
						float haptic_amount = 0.03f + total_knockback / 100.0f;
						if (haptic_amount > 1) {
							haptic_amount = 1;
						}
						//SDL_HapticRumblePlay(haptics[k], haptic_amount, 160);
					}
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
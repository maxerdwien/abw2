#include <string>
#include <SDL.h>
#include <SDL_mixer.h>

#include "asteroid.h"

#include "spaceship.h"
#include "grizzly.h"
#include "renderer.h"

#include "item.h"

#include "bullet.h"
#include "missile.h"

Grizzly::Grizzly(int identifier, int x, int y, Renderer* rend) {
	id = identifier;

	x_pos = x;
	y_pos = y;

	respawn_x = x;
	respawn_y = y;

	stamina_max = 1000;
	stamina = stamina_max;
	stamina_per_frame = 5;

	max_accel = 7000;

	radius = 40 * 10000;
	normal_radius = radius;
	weight = 110;

	r = rend;

	if (id == 0) {
		ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\grizzly-red.png");
		bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletRed.png");
		mine_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\mineRed.png");
	} else if (id == 1) {
		ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\grizzly-blue.png");
		bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletBlue.png");
		mine_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\mineBlue.png");
	} else if (id == 2) {
		ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\grizzly-yellow.png");
		bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletYellow.png");
		mine_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\mineYellow.png");
	} else {
		ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\grizzly-green.png");
		bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletGreen.png");
		mine_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\mineGreen.png");
	}

	ship_invincible_tex = r->LoadTexture("..\\Project1\\assets\\ships\\grizzly-white.png");

	cannon_tex = r->LoadTexture("..\\Project1\\assets\\cannon.png");

	bounce_missile_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\missileOrange.png");
	bounce_bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletOrange.png");
	
	missile_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\missile.png");
	explosion_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\explosion.png");
	shield_tex = r->LoadTexture("..\\Project1\\assets\\shield.png");
	SDL_SetTextureAlphaMod(shield_tex, 100); 
	bounce_tex = r->LoadTexture("..\\Project1\\assets\\bouncer.png");
	SDL_SetTextureAlphaMod(bounce_tex, 100);


	bullet_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\bullet.wav");
	missile_launch_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\missile.wav");
	explosion_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\explosion.wav");
	mine_deploy_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\dispenser.wav");
}

Grizzly::~Grizzly() {
	SDL_DestroyTexture(ship_tex);
	SDL_DestroyTexture(bullet_tex);
	SDL_DestroyTexture(ship_invincible_tex);
	SDL_DestroyTexture(cannon_tex);
	SDL_DestroyTexture(missile_tex);
	SDL_DestroyTexture(explosion_tex);
	SDL_DestroyTexture(bounce_missile_tex);
	SDL_DestroyTexture(bounce_bullet_tex);
	SDL_DestroyTexture(shield_tex);
	SDL_DestroyTexture(bounce_tex);

	Mix_FreeChunk(explosion_sfx);
	Mix_FreeChunk(bullet_sfx);
	Mix_FreeChunk(missile_launch_sfx);
	Mix_FreeChunk(mine_deploy_sfx);

	for (int i = 0; i < num_bullets; i++) {
		delete bullets[i];
	}

	for (int i = 0; i < num_missiles; i++) {
		delete missiles[i];
	}
}

void Grizzly::update() {
	gun_dir_x = desired_gun_dir_x;
	gun_dir_y = desired_gun_dir_y;
}

void Grizzly::fire_1() {
	if (cannon_cooldown > 0) {
		cannon_cooldown--;
	}
	if (do_fire_1 && stamina > 0 && cannon_cooldown <= 0) {
		int MUZZLE_VEL = 100000;
		int spread = 1;
		double angle = atan2(gun_dir_y, gun_dir_x);
		bullet** new_bullets = spawn_bullets(gun_dir_x, gun_dir_y, x_pos+gun_length*cos(angle), y_pos+gun_length*sin(angle), MUZZLE_VEL, spread, 5, 10, 400);
		for (int i = 0; i < spread; i++) {
			bullets[num_bullets] = new_bullets[i];
			num_bullets++;
		}
		free(new_bullets);
		cannon_cooldown += cannon_delay;
		stamina -= 80;
		Mix_PlayChannel(-1, bullet_sfx, 0);
	}
}

void Grizzly::update_projectiles_1(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
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
				delete bullets[j];
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
				delete bullets[j];
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

				bool hit = ships[k]->take_knockback(bullet->x_vel, bullet->y_vel, bullet->base_knockback, bullet->knockback_scaling, bullet->damage, haptics[k]);
				if (hit) {
					damage_done += bullet->damage;
					ships[k]->last_hit = id;
				}

				// delete bullet
				num_bullets--;
				delete bullets[j];
				bullets[j] = bullets[num_bullets];
				j--;
				break;
			}
		}
	}
}

void Grizzly::render_projectiles_1() {
	for (int j = 0; j < num_bullets; j++) {
		double angle = r->atan2_degrees(bullets[j]->x_vel, bullets[j]->y_vel);
		SDL_Texture* tex;
		if (item_times[bullet_bounce] > 0) {
			tex = bounce_bullet_tex;
		} else {
			tex = bullet_tex;
		}
		r->render_texture(tex, bullets[j]->x_pos, bullets[j]->y_pos, angle, 1);
	}
}

void Grizzly::fire_2() {
	if (missile_cooldown > 0) {
		missile_cooldown--;
	}
	if (do_fire_2 && stamina > 0 && missile_cooldown <= 0) {
		int MUZZLE_VEL = 70000;
		int spread = 1;
		double angle = atan2(gun_dir_y, gun_dir_x);
		missile** new_missiles = spawn_missiles(gun_dir_x, gun_dir_y, x_pos + gun_length*cos(angle), y_pos + gun_length*sin(angle), MUZZLE_VEL, spread, 25, 200, 300);
		for (int i = 0; i < spread; i++) {
			missiles[num_missiles] = new_missiles[i];
			num_missiles++;
		}
		free(new_missiles);
		missile_cooldown += missile_delay;
		stamina -= 400;
		Mix_PlayChannel(-1, missile_launch_sfx, 0);
	}
}

void Grizzly::update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
	for (int j = 0; j < num_missiles; j++) {
		struct missile* missile = missiles[j];

		if (!missile->exploded) {
			// find the closest enemy player. this is the one to lock on to.
			double min_dist = 99999999;
			int target_player = -1;
			for (int k = 0; k < 4; k++) {
				if (!ships[k]) continue;
				if (ships[k]->id == id) continue;
				if (ships[k]->lives == 0) continue;
				double dist = sqrt(pow(missile->x_pos - ships[k]->x_pos, 2) + pow(missile->y_pos - ships[k]->y_pos, 2));
				if (dist < min_dist) {
					min_dist = dist;
					target_player = k;
				}
			}
			

			// accelerate towards target player
			if (target_player != -1) {
				double delta_x = ships[target_player]->x_pos - missile->x_pos;
				double delta_y = ships[target_player]->y_pos - missile->y_pos;
				double delta_mag = sqrt(pow(delta_x, 2) + pow(delta_y, 2));

				missile->x_accel = MISSILE_ACCEL * delta_x / delta_mag;
				missile->y_accel = MISSILE_ACCEL * delta_y / delta_mag;

				missile->x_vel += missile->x_accel;
				missile->y_vel += missile->y_accel;
			}
			missile->x_pos += missile->x_vel;
			missile->y_pos += missile->y_vel;
		}

		// check for missile going out of bounds
		if (missile->x_pos < min_x || missile->x_pos > max_x || missile->y_pos < min_y || missile->y_pos > max_y) {
			if (item_times[bullet_bounce] > 0) {
				if (missile->x_pos < min_x && missile->x_vel < 0) {
					missile->x_vel *= -1;
				} else if (missile->x_pos > max_x && missile->x_vel > 0) {
					missile->x_vel *= -1;
				} else if (missile->y_pos < min_y && missile->y_vel < 0) {
					missile->y_vel *= -1;
				} else if (missile->y_pos > max_y && missile->y_vel > 0) {
					missile->y_vel *= -1;
				}
			} else {
				num_missiles--;
				delete missiles[j];
				missiles[j] = missiles[num_missiles];
				j--;
				continue;
			}
		}

		

		// expand missile radius
		if (missile->exploded) {
			missile->radius += MISSILE_RADIUS_PER_FRAME;
			if (missile->radius > MISSILE_MAX_RADIUS) {
				num_missiles--;
				delete missiles[j];
				missiles[j] = missiles[num_missiles];
				j--;
				continue;
			}
		} else {
			// check for collisions with asteroids
			for (int k = 0; k < num_asteroids; k++) {
				Asteroid* a = asteroids[k];
				double dist = sqrt(pow(a->x_pos - missile->x_pos, 2) + pow(a->y_pos - missile->y_pos, 2));
				if (dist <= (missile->radius + a->radius)) {
					// todo: make bullets bounce if they have the powerup
					missile->exploded = true;
					Mix_PlayChannel(-1, explosion_sfx, 0);
					continue;
				}
			}
		}

		// check for collisions with enemies
		for (int k = 0; k < 4; k++) {
			if (!ships[k]) continue;
			if (ships[k]->lives == 0) continue;
			double dist = sqrt(pow(missile->x_pos - ships[k]->x_pos, 2) + pow(missile->y_pos - ships[k]->y_pos, 2));
			if (!missile->exploded) {
				if (ships[k]->id == id) continue;
				if (dist <= MISSILE_ACTIVATION_RADIUS) {
					missile->exploded = true;
					missile->x_vel = 0;
					missile->y_vel = 0;
					Mix_PlayChannel(-1, explosion_sfx, 0);
				}
			} else {
				if (dist <= (ships[k]->radius + missile->radius)) {
					if (missile->players_hit[k]) continue;
					missile->players_hit[k] = true;

					bool hit = ships[k]->take_knockback(ships[k]->x_pos - missile->x_pos, ships[k]->y_pos - missile->y_pos, missile->base_knockback, missile->knockback_scaling, missile->damage, haptics[k]);
					if (hit) {
						damage_done += missile->damage;
						ships[k]->last_hit = id;
					}
				}
			}
		}
	}
}

void Grizzly::render_projectiles_2() {
	for (int j = 0; j < num_missiles; j++) {
		if (!missiles[j]->exploded) {
			double angle = r->atan2_degrees(missiles[j]->x_vel, missiles[j]->y_vel);
			SDL_Texture* tex;
			if (item_times[bullet_bounce] > 0) {
				tex = bounce_missile_tex;
			} else {
				tex = missile_tex;
			}
			r->render_texture(tex, missiles[j]->x_pos, missiles[j]->y_pos, angle, 1.8);
		} else {
			r->render_texture_abs_size(explosion_tex, missiles[j]->x_pos, missiles[j]->y_pos, 0, missiles[j]->radius);
		}
	}
}

void Grizzly::fire_3() {
	if (mine_cooldown > 0) {
		mine_cooldown--;
	}
	if (do_fire_3 && stamina > 0 && mine_cooldown <= 0) {
		int MUZZLE_VEL = 1000;
		int spread = 1;
		double angle = atan2(gun_dir_y, gun_dir_x);
		missile** new_missiles = spawn_missiles(gun_dir_x, gun_dir_y, x_pos+gun_length*cos(angle), y_pos+gun_length*sin(angle), MUZZLE_VEL, spread, 35, 200, 300);
		for (int i = 0; i < spread; i++) {
			mines[num_mines] = new_missiles[i];
			num_mines++;
		}
		free(new_missiles);
		mine_cooldown += mine_delay;
		stamina -= 550;
		Mix_PlayChannel(-1, mine_deploy_sfx, 0);
	}
}

void Grizzly::update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
	for (int j = 0; j < num_mines; j++) {
		struct missile* mine = mines[j];

		if (!mine->exploded) {
			// find the closest enemy player. this is the one to lock on to.
			double min_dist = 10000 * 200;
			int target_player = -1;
			for (int k = 0; k < 4; k++) {
				if (!ships[k]) continue;
				if (ships[k]->id == id) continue;
				if (ships[k]->lives == 0) continue;
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
			delete mines[j];
			mines[j] = mines[num_mines];
			j--;
			continue;
		}

		// expand missile radius
		if (mine->exploded) {
			mine->radius += MISSILE_RADIUS_PER_FRAME;
			if (mine->radius > MISSILE_MAX_RADIUS) {
				num_mines--;
				delete mines[j];
				mines[j] = mines[num_mines];
				j--;
				continue;
			}
		} else {
			// check for collisions with asteroids
			for (int k = 0; k < num_asteroids; k++) {
				Asteroid* a = asteroids[k];
				double dist = sqrt(pow(a->x_pos - mine->x_pos, 2) + pow(a->y_pos - mine->y_pos, 2));
				if (dist <= (mine->radius + a->radius)) {
					// todo: make bullets bounce if they have the powerup
					mine->exploded = true;
					continue;
				}
			}
		}
		// check for collisions with enemies
		for (int k = 0; k < 4; k++) {
			if (!ships[k]) continue;
			if (ships[k]->lives == 0) continue;

			double dist = sqrt(pow(mine->x_pos - ships[k]->x_pos, 2) + pow(mine->y_pos - ships[k]->y_pos, 2));
			if (!mine->exploded) {
				if (ships[k]->id == id) continue;
				if (dist <= MISSILE_ACTIVATION_RADIUS) {
					mine->exploded = true;
					mine->x_vel = 0;
					mine->y_vel = 0;
					Mix_PlayChannel(-1, explosion_sfx, 0);
				}
			} else {
				if (dist <= (ships[k]->radius + mine->radius)) {
					if (mine->players_hit[k]) continue;
					mine->players_hit[k] = true;

					bool hit = ships[k]->take_knockback(ships[k]->x_pos - mine->x_pos, ships[k]->y_pos - mine->y_pos, mine->base_knockback, mine->knockback_scaling, mine->damage, haptics[k]);
					if (hit) {
						damage_done += mine->damage;
						ships[k]->last_hit = id;
					}
				}
			}
		}
	}
}

void Grizzly::render_projectiles_3() {
	for (int j = 0; j < num_mines; j++) {
		if (!mines[j]->exploded) {
			double angle = r->atan2_degrees(mines[j]->x_vel, mines[j]->y_vel);
			r->render_texture(mine_tex, mines[j]->x_pos, mines[j]->y_pos, angle, 1.8);
		} else {
			r->render_texture_abs_size(explosion_tex, mines[j]->x_pos, mines[j]->y_pos, 0, mines[j]->radius);
		}
	}
}
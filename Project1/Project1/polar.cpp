#include <string>
#include <SDL.h>
#include <SDL_mixer.h>

#include "serializer.h"

#include "asteroid.h"
#include "spaceship.h"
#include "spark.h"
#include "gravity-missile.h"
#include "polar.h"
#include "renderer.h"

#include "item.h"

#include "bullet.h"


Polar::Polar(int identifier, int a1, int a2, int x, int y, Renderer* rend) {
	id = identifier;

	ally1 = a1;
	ally2 = a2;

	x_pos = x;
	y_pos = y;

	respawn_x = x;
	respawn_y = y;

	stamina_max = 1600;
	stamina = stamina_max;
	stamina_per_frame = 4;

	max_accel = 5500;

	radius = 40 * 10000;
	normal_radius = radius;
	weight = 130;

	laser_channel = 24 + 2*id;

	r = rend;

	if (id == 0) {
		ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\polar-red.png");
		bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletRed.png");
		missile_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\missileRed.png");
	} else if (id == 1) {
		if (ally1 == -1) {
			ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\polar-blue.png");
			bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletBlue.png");
			missile_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\missileBlue.png");
		} else {
			ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\polar-magenta.png");
			bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletMagenta.png");
			missile_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\missileMagenta.png");
		}
	} else if (id == 2) {
		if (ally1 == -1) {
			ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\polar-yellow.png");
			bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletYellow.png");
			missile_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\missileYellow.png");
		} else {
			ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\polar-blue.png");
			bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletBlue.png");
			missile_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\missileBlue.png");
		}
	} else {
		if (ally1 == -1) {
			ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\polar-green.png");
			bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletGreen.png");
			missile_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\missileGreen.png");
		} else {
			ship_tex = r->LoadTexture("..\\Project1\\assets\\ships\\polar-teal.png");
			bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletTeal.png");
			missile_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\missileTeal.png");
		}
	}

	ship_invincible_tex = r->LoadTexture("..\\Project1\\assets\\ships\\polar-white.png");

	cannon_tex = r->LoadTexture("..\\Project1\\assets\\cannon.png");

	bounce_missile_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\missileOrange.png");
	bounce_bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletOrange.png");

	vortex_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\blackhole.png");
	SDL_SetTextureAlphaMod(vortex_tex, 100);

	shield_tex = r->LoadTexture("..\\Project1\\assets\\shield.png");
	SDL_SetTextureAlphaMod(shield_tex, 100);
	bounce_tex = r->LoadTexture("..\\Project1\\assets\\bouncer.png");
	SDL_SetTextureAlphaMod(bounce_tex, 100);

	thrust_low_tex = r->LoadTexture("..\\Project1\\assets\\thrustlow.png");
	thrust_medium_tex = r->LoadTexture("..\\Project1\\assets\\thrustmed.png");
	thrust_high_tex = r->LoadTexture("..\\Project1\\assets\\thrusthigh.png");

	laser_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\laser-active-big.wav");
	bullet_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\bullet.wav");
	missile_launch_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\missile.wav");
	blackhole_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\blackhole.wav");
}

Polar::~Polar() {
	SDL_DestroyTexture(ship_tex);
	SDL_DestroyTexture(bullet_tex);
	SDL_DestroyTexture(ship_invincible_tex);
	SDL_DestroyTexture(cannon_tex);
	SDL_DestroyTexture(missile_tex);
	SDL_DestroyTexture(vortex_tex);
	SDL_DestroyTexture(bounce_missile_tex);
	SDL_DestroyTexture(bounce_bullet_tex);
	SDL_DestroyTexture(shield_tex);
	SDL_DestroyTexture(bounce_tex);
	SDL_DestroyTexture(thrust_low_tex);
	SDL_DestroyTexture(thrust_medium_tex);
	SDL_DestroyTexture(thrust_high_tex);

	Mix_FreeChunk(laser_sfx);
	Mix_FreeChunk(bullet_sfx);
	Mix_FreeChunk(missile_launch_sfx);
	Mix_FreeChunk(blackhole_sfx);

	for (int i = 0; i < num_bullets; i++) {
		delete bullets[i];
	}
	for (int i = 0; i < num_sparks; i++) {
		delete sparks[i];
	}

	for (int i = 0; i < num_g_missiles; i++) {
		delete g_missiles[i];
	}
}

void Polar::update() {
	double angle = atan2(gun_dir_y, gun_dir_x);
	double desired_angle = atan2(desired_gun_dir_y, desired_gun_dir_x);
	double new_angle = angle;

	double angle_diff = desired_angle - angle;

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

	gun_dir_x = (int)(10000 * cos(new_angle));
	gun_dir_y = (int)(10000 * sin(new_angle));
}

void Polar::die() {
	Mix_HaltChannel(laser_channel);
}

void Polar::fire_1() {
	// handle spread fire spawns
	if (spread_cooldown > 0) {
		spread_cooldown--;
	}
	if (do_fire_1 && stamina > 0 && spread_cooldown <= 0) {

		int MUZZLE_VEL = 40000;
		int spread = 7;
		double angle = atan2(gun_dir_y, gun_dir_x);
		Bullet** new_bullets = spawn_spread_bullets(gun_dir_x, gun_dir_y, (int)(x_pos + gun_length*cos(angle)), (int)(y_pos + gun_length*sin(angle)), MUZZLE_VEL, spread, M_PI * 0.1, 5, 10, 100);
		for (int i = 0; i < spread; i++) {
			bullets[num_bullets] = new_bullets[i];
			num_bullets++;
		}
		free(new_bullets);
		spread_cooldown += spread_delay;
		stamina -= 250;

		Mix_PlayChannel(-1, bullet_sfx, 0);
	}
}

void Polar::update_projectiles_1(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
	for (int j = 0; j < num_bullets; j++) {
		Bullet* bullet = bullets[j];

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
			if (ships[k]->id == ally1) continue;
			if (ships[k]->id == ally2) continue;
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

void Polar::render_projectiles_1() {
	for (int j = 0; j < num_bullets; j++) {
		double angle = r->atan2_degrees(bullets[j]->x_vel, bullets[j]->y_vel);
		SDL_Texture* tex;
		if (item_times[bullet_bounce] > 0) {
			tex = bounce_bullet_tex;
		} else {
			tex = bullet_tex;
		}
		if (r->render_normal) {
			r->render_texture(tex, bullets[j]->x_pos, bullets[j]->y_pos, angle, 1);
		}
		if (r->render_debug) {
			r->render_texture_abs_size(r->hitbox_tex, bullets[j]->x_pos, bullets[j]->y_pos, 0, bullets[j]->radius);
		}
	}
}

void Polar::fire_2() {
	if (missile_cooldown > 0) {
		missile_cooldown--;
	}

	if (!do_fire_2 && missile_click_used) {
		for (int i = 0; i < num_g_missiles; i++) {
			if (!g_missiles[i]->exploded) {
				g_missiles[i]->exploded = true;
				// todo: maybe it keeps moving a little bit?
				g_missiles[i]->x_vel = 0;
				g_missiles[i]->y_vel = 0;
				Mix_PlayChannel(-1, blackhole_sfx, 0);
				missile_click_used = false;
			}
		}
	}

	if (do_fire_2 && !missile_click_used && stamina > 0 && missile_cooldown <= 0) {
		double angle = atan2(gun_dir_y, gun_dir_x);
		int velocity = 100000;
		int x_vel = (int)(cos(angle) * velocity);
		int y_vel = (int)(sin(angle) * velocity);
		g_missiles[num_g_missiles] = new Gravity_Missile(x_pos + (int)(gun_length*cos(angle)), (int)(y_pos+gun_length*sin(angle)), x_vel, y_vel);
		num_g_missiles++;
		missile_cooldown += missile_delay;
		stamina -= 600;
		Mix_PlayChannel(-1, missile_launch_sfx, 0);
		missile_click_used = true;
	}
}

void Polar::update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
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
				delete g_missiles[i];
				g_missiles[i] = g_missiles[num_g_missiles];
				i--;
				continue;
			}
		} else {
			// check for collisions with asteroids
			for (int k = 0; k < num_asteroids; k++) {
				Asteroid* a = asteroids[k];
				double dist = sqrt(pow(a->x_pos - m->x_pos, 2) + pow(a->y_pos - m->y_pos, 2));
				if (dist <= (m->radius + a->radius)) {
					// todo: make bullets bounce if they have the powerup
					m->exploded = true;
					m->x_vel = 0;
					m->y_vel = 0;
					Mix_PlayChannel(-1, blackhole_sfx, 0);
					missile_click_used = false;
					continue;
				}
			}
		}
		

		m->x_pos += m->x_vel;
		m->y_pos += m->y_vel;

		// check for missile going out of bounds
		if (m->x_pos < min_x || m->x_pos > max_x || m->y_pos < min_y || m->y_pos > max_y) {
			if (item_times[bullet_bounce] > 0) {
				if (m->x_pos < min_x && m->x_vel < 0) {
					m->x_vel *= -1;
				} else if (m->x_pos > max_x && m->x_vel > 0) {
					m->x_vel *= -1;
				} else if (m->y_pos < min_y && m->y_vel < 0) {
					m->y_vel *= -1;
				} else if (m->y_pos > max_y && m->y_vel > 0) {
					m->y_vel *= -1;
				}
			} else {
				missile_click_used = false;
				num_g_missiles--;
				delete g_missiles[i];
				g_missiles[i] = g_missiles[num_g_missiles];
				i--;
				continue;
			}
			
		}

		// increment damage counter
		m->damage_counter = (m->damage_counter + 1) % 3;

		// check for collisions with enemies
		for (int k = 0; k < 4; k++) {
			if (!ships[k]) continue;
			if (ships[k]->lives == 0) continue;
			double dist = sqrt(pow(m->x_pos - ships[k]->x_pos, 2) + pow(m->y_pos - ships[k]->y_pos, 2));
			if (!m->exploded) {
				if (ships[k]->id == id) continue;
				if (ships[k]->id == ally1) continue;
				if (ships[k]->id == ally2) continue;
				if (dist <= (ships[k]->radius + m->radius)) {
					m->exploded = true;
					m->x_vel = 0;
					m->y_vel = 0;
					Mix_PlayChannel(-1, blackhole_sfx, 0);
					missile_click_used = false;
				}
			} else {
				if (dist <= (ships[k]->radius + m->radius)) {
					if (m->damage_counter % 3 == 0) {
						bool hit = ships[k]->take_knockback(ships[k]->x_pos - m->x_pos, ships[k]->y_pos - m->y_pos, m->base_knockback, m->knockback_scaling, m->damage, haptics[k]);
						if (hit) {
							damage_done += m->damage;
							ships[k]->last_hit = id;
						}
					}
				}

				// do gravity effect
				if (ships[k]->invincibility_cooldown == 0) {
					double force = 10000000000000000.0 / pow(dist, 2);
					if (force > 70000) force = 70000;
					double angle = atan2(m->y_pos - ships[k]->y_pos, m->x_pos - ships[k]->x_pos);
					ships[k]->x_vel += (int)(force * cos(angle));
					ships[k]->y_vel += (int)(force * sin(angle));
				}
			}
		}
	}
}

void Polar::render_projectiles_2() {
	for (int j = 0; j < num_g_missiles; j++) {
		if (!g_missiles[j]->exploded) {
			double angle = r->atan2_degrees(g_missiles[j]->x_vel, g_missiles[j]->y_vel);
			SDL_Texture* tex;
			if (item_times[bullet_bounce] > 0) {
				tex = bounce_missile_tex;
			} else {
				tex = missile_tex;
			}
			if (r->render_normal) {
				r->render_texture(tex, g_missiles[j]->x_pos, g_missiles[j]->y_pos, angle, 3);
			}
			if (r->render_debug) {
				r->render_texture_abs_size(r->activation_hitbox_tex, g_missiles[j]->x_pos, g_missiles[j]->y_pos, 0, g_missiles[j]->radius);
			}
			
		} else {
			if (r->render_normal) {
				r->render_texture_abs_size(vortex_tex, g_missiles[j]->x_pos, g_missiles[j]->y_pos, g_missiles[j]->vortex_angle, g_missiles[j]->radius);
				r->render_texture_abs_size(vortex_tex, g_missiles[j]->x_pos, g_missiles[j]->y_pos, -g_missiles[j]->vortex_angle, g_missiles[j]->radius);
				r->render_texture_abs_size(vortex_tex, g_missiles[j]->x_pos, g_missiles[j]->y_pos, g_missiles[j]->vortex_angle + 45, g_missiles[j]->radius);
				r->render_texture_abs_size(vortex_tex, g_missiles[j]->x_pos, g_missiles[j]->y_pos, -g_missiles[j]->vortex_angle + 45, g_missiles[j]->radius);
			}
			if (r->render_debug) {
				r->render_texture_abs_size(r->hitbox_tex, g_missiles[j]->x_pos, g_missiles[j]->y_pos, 0, g_missiles[j]->radius);
			}
			
		}
	}
}

void Polar::fire_3() {
	if (do_fire_3 && stamina > 0) {
		laser_active = true;
		stamina -= 16;
	} else {
		laser_active = false;
		do_fire_3 = false;
	}
}

void Polar::update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
	if (laser_active) {
		// update laser position
		double angle = atan2(gun_dir_y, gun_dir_x);
		laser_start_x = (int)(x_pos + gun_length * cos(angle));
		laser_start_y = (int)(y_pos + gun_length * sin(angle));
		laser_end_x = laser_start_x + 10000*gun_dir_x;
		laser_end_y = laser_start_y + 10000*gun_dir_y;

		for (int i = 0; i < 4; i++) {
			if (!ships[i]) continue;
			if (ships[i]->id == id) continue;
			if (ships[i]->id == ally1) continue;
			if (ships[i]->id == ally2) continue;
			if (ships[i]->lives == 0) continue;
			Ship* target_ship = ships[i];
			double dist = get_dist(laser_start_x, laser_start_y, laser_end_x, laser_end_y, target_ship->x_pos, target_ship->y_pos);

			if (dist < target_ship->radius) {
				if (abs(laser_end_x - laser_start_x) > abs(laser_end_y - laser_start_y)) {
					bool x_mismatch = target_ship->x_pos - x_pos > 0 && laser_end_x - laser_start_x < 0 || target_ship->x_pos - x_pos < 0 && laser_end_x - laser_start_x > 0;
					if (x_mismatch) continue;
				} else {
					bool y_mismatch = target_ship->y_pos - y_pos > 0 && laser_end_y - laser_start_y < 0 || target_ship->y_pos - y_pos < 0 && laser_end_y - laser_start_y > 0;
					if (y_mismatch) continue;
				}

				double ship_dist = sqrt(pow(target_ship->x_pos - x_pos, 2) + pow(target_ship->y_pos - y_pos, 2));

				int damage_amount = 0;
				if (laser_damage_cooldown == 0) {
					laser_damage_cooldown = laser_damage_delay;
					damage_amount = 1;
				} else {
					laser_damage_cooldown--;
				}
				
				bool hit = target_ship->take_knockback(laser_end_x - laser_start_x, laser_end_y - laser_start_y, 0, 30+(int)(ship_dist/1000000), damage_amount, haptics[i]);
				if (hit) {
					damage_done += damage_amount;
					target_ship->last_hit = id;
					sparks[num_sparks] = new Spark(target_ship->x_pos, target_ship->y_pos);
					num_sparks++;
				}
			}
		}

		// recoil. from a laser. I guess that makes sense.
		take_knockback(laser_start_x - laser_end_x, laser_start_y - laser_end_y, 5, 10, 0, haptics[id]);
		
	}

	// update sparks
	for (int i = 0; i < num_sparks; i++) {
		Spark* s = sparks[i];
		double angle = atan2(s->y_2 - s->y_1, s->x_2 - s->x_1);
		s->x_1 += (int)(cos(angle) * s->vel);
		s->y_1 += (int)(sin(angle) * s->vel);
		s->x_2 += (int)(cos(angle) * s->vel);
		s->y_2 += (int)(sin(angle) * s->vel);

		s->remaining_life--;
		if (s->remaining_life == 0) {
			delete sparks[i];
			sparks[i] = sparks[num_sparks-1];
			num_sparks--;
			i--;
		}
	}
}

void Polar::render_projectiles_3() {
	if (laser_active) {
		r->SetRenderDrawColor(128, 0, 0, SDL_ALPHA_OPAQUE);
		r->render_line_thick(laser_start_x, laser_start_y, gun_dir_x, gun_dir_y);

		if (!Mix_Playing(laser_channel)) {
			Mix_PlayChannel(laser_channel, laser_sfx, 0);
		}
	} else {
		Mix_HaltChannel(laser_channel);
	}

	for (int i = 0; i < num_sparks; i++) {
		Spark* s = sparks[i];
		r->SetRenderDrawColor(255, 255, 0, SDL_ALPHA_OPAQUE);
		if (r->render_normal) {
			r->render_line_w_end(s->x_1, s->y_1, s->x_2, s->y_2);
		}
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

int Polar::serialize(char* buf, int i) {
	i = serialize_ship(buf, i);

	i = serialize_int(num_bullets, buf, i);
	for (int j = 0; j < num_bullets; j++) {
		i = bullets[j]->serialize(buf, i);
	}

	i = serialize_int(num_g_missiles, buf, i);
	for (int j = 0; j < num_g_missiles; j++) {
		i = g_missiles[j]->serialize(buf, i);
	}

	i = serialize_bool(laser_active, buf, i);

	i = serialize_int(laser_start_x, buf, i);
	i = serialize_int(laser_start_y, buf, i);
	i = serialize_int(laser_end_x, buf, i);
	i = serialize_int(laser_end_y, buf, i);

	i = serialize_int(num_sparks, buf, i);
	for (int j = 0; j < num_sparks; j++) {
		i = sparks[j]->serialize(buf, i);
	}

	return i;
}

int Polar::deserialize(char*buf, int i) {
	i = deserialize_ship(buf, i);

	i = deserialize_int(&num_bullets, buf, i);
	for (int j = 0; j < num_bullets; j++) {
		bullets[j] = new Bullet();
		i = bullets[j]->deserialize(buf, i);
	}

	i = deserialize_int(&num_g_missiles, buf, i);
	for (int j = 0; j < num_g_missiles; j++) {
		g_missiles[j] = new Gravity_Missile();
		i = g_missiles[j]->deserialize(buf, i);
	}

	i = deserialize_bool(&laser_active, buf, i);

	i = deserialize_int(&laser_start_x, buf, i);
	i = deserialize_int(&laser_start_y, buf, i);
	i = deserialize_int(&laser_end_x, buf, i);
	i = deserialize_int(&laser_end_y, buf, i);

	i = deserialize_int(&num_sparks, buf, i);
	for (int j = 0; j < num_sparks; j++) {
		sparks[j] = new Spark();
		i = sparks[j]->deserialize(buf, i);
	}

	return i;
}
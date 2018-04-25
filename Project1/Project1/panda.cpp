#include <string>
#include <SDL.h>
#include <SDL_mixer.h>

#include "serializer.h"

#include "asteroid.h"
#include "spaceship.h"
#include "spark.h"
#include "squiggle_segment.h"
#include "gravity-missile.h"
#include "panda.h"
#include "renderer.h"

#include "item.h"

#include "bullet.h"


Panda::Panda(int identifier, int a1, int a2, int x, int y, Renderer* rend) {
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

	laser_channel = 24 + 2 * id;

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

	bounce_bullet_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\bulletOrange.png");

	squiggle_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\squiggle_segment_red.png");
	squiggle_circle_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\squiggle_circle_red.png");
	squiggle_rect_tex = r->LoadTexture("..\\Project1\\assets\\attacks\\squiggle_rectangle_red.png");
	//SDL_SetTextureAlphaMod(vortex_tex, 100);

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

Panda::~Panda() {
	SDL_DestroyTexture(ship_tex);
	SDL_DestroyTexture(bullet_tex);
	SDL_DestroyTexture(ship_invincible_tex);
	SDL_DestroyTexture(cannon_tex);
	SDL_DestroyTexture(missile_tex);
	SDL_DestroyTexture(squiggle_tex);
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

void Panda::update() {
	gun_dir_x = desired_gun_dir_x;
	gun_dir_y = desired_gun_dir_y;
}

void Panda::die() {
	Mix_HaltChannel(laser_channel);
}

void Panda::fire_1() {
	// handle spread fire spawns
	if (spread_cooldown > 0) {
		spread_cooldown--;
	}

	// todo: how do i want to handle stamina for this attack?
	if (do_fire_1 && stamina > 0 && spread_cooldown <= 0) {
		// takes stamina to begin charging, but not to continue charging
		if (frames_charging_spread == 0) {
			stamina -= 50;
		}
		frames_charging_spread++;
	}

	if (!do_fire_1 && frames_charging_spread > 0) {
		double spread_angle = (M_PI / 2) - atan(frames_charging_spread / 5.0);
		int MUZZLE_VEL = 80000;
		int spread = 8;
		double angle = atan2(gun_dir_y, gun_dir_x);
		Bullet** new_bullets = spawn_spread_bullets(gun_dir_x, gun_dir_y, (int)(x_pos + gun_length*cos(angle)), (int)(y_pos + gun_length*sin(angle)), MUZZLE_VEL, spread, spread_angle, 5, 10, 100);
		for (int i = 0; i < spread; i++) {
			bullets[num_bullets] = new_bullets[i];
			num_bullets++;
		}
		free(new_bullets);
		spread_cooldown += spread_delay;
		stamina -= 300;
		Mix_PlayChannel(-1, bullet_sfx, 0);

		frames_charging_spread = 0;
	}
}

void Panda::update_projectiles_1(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
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

void Panda::render_projectiles_1() {
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

void Panda::fire_2() {
	if (do_fire_2) {
		double angle = atan2(gun_dir_y, gun_dir_x);

		if (squiggle_state == squiggle_state::inactive) {
			//max_accel = 0;
			// todo: temporary
			//x_vel = 0;
			//y_vel = 0;

			squiggle_state = squiggle_state::shooting;
			

			//squiggle_x = (int)(x_pos + gun_length*cos(angle));
			//squiggle_y = (int)(y_pos + gun_length*sin(angle));
			squiggle_x = x_pos;
			squiggle_y = y_pos;

			
		}
		
		squiggle_angle = angle;
	} else {
		if (squiggle_state == squiggle_state::shooting) {
			squiggle_state = squiggle_state::growing;
			max_accel = 5500;
		}
	}
}

void Panda::update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
	if (squiggle_state == squiggle_state::shooting) {
		if (stamina > 0) {
			if (!num_squiggle_segments) {
				squiggle_segments[num_squiggle_segments] = new Squiggle_Segment(squiggle_x, squiggle_y); // todo: leak
				num_squiggle_segments++;
			}
			// make new squiggle segments
			double angle = atan2(gun_dir_y, gun_dir_x);

			// todo: leak
			for (int i = 0; i < squiggle_segs_per_frame; i++) {
				squiggle_x += squiggle_dist_per_frame * cos(angle) / squiggle_segs_per_frame;
				squiggle_y += squiggle_dist_per_frame * sin(angle) / squiggle_segs_per_frame;

				squiggle_segments[num_squiggle_segments] = new Squiggle_Segment(squiggle_x, squiggle_y);
				num_squiggle_segments++;
			}

			stamina -= 25;
		} else {
			squiggle_state == squiggle_state::growing;
		}
	} else if (squiggle_state == squiggle_state::growing) {
		// change size
		if (squiggle_width < max_squiggle_width) {
			squiggle_width += squiggle_width_per_frame;
		} else {
			squiggle_state = squiggle_state::shrinking;
		}
	} else if (squiggle_state == squiggle_state::shrinking) {
		squiggle_width -= squiggle_shrinkage_rate;

		if (squiggle_width <= 0) {
			squiggle_state = squiggle_state::inactive;
		}
	}
	else if (squiggle_state == squiggle_state::inactive) {
		for (int i = 0; i < num_squiggle_segments; i++) {
			delete squiggle_segments[i];
		}
		num_squiggle_segments = 0;

		squiggle_width = normal_squiggle_width;
	}

	

	// pull squiggle
	if (num_squiggle_segments) {
		Squiggle_Segment* seg = squiggle_segments[0];
		int delta_x = x_pos - seg->x;
		int delta_y = y_pos - seg->y;

		seg->x += delta_x;
		seg->y += delta_y;


		for (int i = 0; i < num_squiggle_segments - 1; i++) {
			Squiggle_Segment* seg_a = squiggle_segments[i];
			Squiggle_Segment* seg_b = squiggle_segments[i+1];

			double angle = atan2(seg_b->y - seg_a->y, seg_b->x - seg_a->x);
			
			seg_b->x = seg_a->x + (int)(squiggle_dist_per_frame*cos(angle) / squiggle_segs_per_frame);
			seg_b->y = seg_a->y + (int)(squiggle_dist_per_frame*sin(angle) / squiggle_segs_per_frame);

			// check for collisions
			if (squiggle_state == squiggle_state::growing || squiggle_state == squiggle_state::shrinking) {
				for (int j = 0; j < 4; j++) {
					if (!ships[j]) continue;
					if (j == id) continue;
					// circles
					// todo: check first circle?
					double dist_squared = pow(seg_b->x - ships[j]->x_pos, 2) + pow(seg_b->y - ships[j]->y_pos, 2);
					double radii_squared = pow(ships[j]->radius + squiggle_width/2, 2);
					if (dist_squared < radii_squared) {
						ships[j]->take_knockback(1000*sin(angle), 1000*cos(angle), 10, 10, 10, haptics[j]);
					}
					// todo: rectangles?
				}
			}
		}
	}
}

void Panda::render_projectiles_2() {
	for (int i = 0; i < num_squiggle_segments; i++) {
		Squiggle_Segment* seg_a = squiggle_segments[i];
		r->render_texture_abs_size(squiggle_circle_tex, seg_a->x, seg_a->y, 0, squiggle_width/2);

		if (i != num_squiggle_segments - 1) {
			Squiggle_Segment* seg_b = squiggle_segments[i + 1];
			//r->render_line_w_end(seg_a->x, seg_a->y, seg_b->x, seg_b->y);
			//double angle = atan2(seg_b->y - seg_a->y, seg_b->x - seg_a->x);
			double angle = r->atan2_degrees(seg_b->x - seg_a->x, seg_b->y - seg_a->y);
			r->render_squiggle_rectangle(squiggle_rect_tex, (seg_a->x + seg_b->x) / 2, (seg_a->y + seg_b->y) / 2, angle, squiggle_width, squiggle_dist_per_frame / squiggle_segs_per_frame);
		}
	}
}

void Panda::fire_3() {
	if (do_fire_3 && stamina > 0) {
		laser_active = true;
		stamina -= 16;
	} else {
		laser_active = false;
		do_fire_3 = false;
	}
}

void Panda::update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) {
	if (laser_active) {
		// update laser position
		double angle = atan2(gun_dir_y, gun_dir_x);
		laser_start_x = (int)(x_pos + gun_length * cos(angle));
		laser_start_y = (int)(y_pos + gun_length * sin(angle));
		laser_end_x = laser_start_x + 10000 * gun_dir_x;
		laser_end_y = laser_start_y + 10000 * gun_dir_y;

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

				bool hit = target_ship->take_knockback(laser_end_x - laser_start_x, laser_end_y - laser_start_y, 0, 30 + (int)(ship_dist / 1000000), damage_amount, haptics[i]);
				if (hit) {
					damage_done += damage_amount;
					target_ship->last_hit = id;
					sparks[num_sparks] = new Spark(target_ship->x_pos, target_ship->y_pos);
					num_sparks++;
				}
			}
		}

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
			sparks[i] = sparks[num_sparks - 1];
			num_sparks--;
			i--;
		}
	}
}

void Panda::render_projectiles_3() {
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

double Panda::get_dist(long x_1, long y_1, long x_2, long y_2, long x_0, long y_0) {
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

int Panda::serialize(char* buf, int i) {
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

int Panda::deserialize(char*buf, int i) {
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
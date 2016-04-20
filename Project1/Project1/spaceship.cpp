#include <SDL.h>
#include <iostream>

#include "asteroid.h"

#include "spaceship.h"
#include "renderer.h"
#include "item.h"

Ship::~Ship() {

}

void Ship::render() {
	double scale = 1;
	if (item_times[small] > 0) {
		scale = 0.5;
	}
	double angle = r->atan2_degrees(face_dir_x, face_dir_y);

	if (invincibility_cooldown > 0) {
		if (invincibility_cooldown % invincibility_switch_rate < (2 *invincibility_switch_rate/3)) {
			r->render_texture(ship_tex, x_pos, y_pos, angle, 3 * scale);
		} else {
			r->render_texture(ship_invincible_tex, x_pos, y_pos, angle, 3 * scale);
		}
	} else {
		r->render_texture(ship_tex, x_pos, y_pos, angle, 3 * scale);
	}

	// render gun
	r->render_texture_edge_spin(cannon_tex, x_pos, y_pos, r->atan2_degrees(gun_dir_x, gun_dir_y), 1.75 * scale);

	// render laser sight
	if (item_times[laser_sights]) {
		if (id == 0) {
			r->SetRenderDrawColor(160, 0, 0, SDL_ALPHA_OPAQUE);
		} else if (id == 1) {
			r->SetRenderDrawColor(0, 0, 160, SDL_ALPHA_OPAQUE);
		} else if (id == 2) {
			r->SetRenderDrawColor(210, 210, 0, SDL_ALPHA_OPAQUE);
		} else {
			r->SetRenderDrawColor(0, 160, 0, SDL_ALPHA_OPAQUE);
		}

		double angle = atan2(gun_dir_y, gun_dir_x);
		double gun_len = gun_length * scale;
		r->render_line(x_pos + gun_len*cos(angle), y_pos + gun_len*sin(angle), gun_dir_x, gun_dir_y);
	}

	if (item_times[shield] > 0) {
		r->render_texture(shield_tex, x_pos, y_pos, angle, 4 * scale);
	}
	if (item_times[bounce] > 0) {
		r->render_texture(bounce_tex, x_pos, y_pos, angle, 4 * scale);
	}
}

bool Ship::take_knockback(int dir_x, int dir_y, int base_knockback, int knockback_scaling, int damage, SDL_Haptic* haptic) {
	if (invincibility_cooldown != 0) return false;
	if (item_times[shield] > 0) return false;

	// knockback
	int total_knockback = (int)(100.0 * (base_knockback + (((double)percent) / 100.0)*knockback_scaling) / weight);
	
	double normalized_dir_x = dir_x / sqrt(pow(dir_x, 2) + pow(dir_y, 2));
	double normalized_dir_y = dir_y / sqrt(pow(dir_x, 2) + pow(dir_y, 2));

	x_vel += (int)(1000.0*total_knockback*normalized_dir_x);
	y_vel += (int)(1000.0*total_knockback*normalized_dir_y);

	// damage
	percent += damage;
	damage_taken += damage;
	if (percent > SPACESHIP_MAX_PERCENT) {
		percent = SPACESHIP_MAX_PERCENT;
	}

	// haptic
	float haptic_amount = 0.03f + total_knockback / 100.0f;
	if (haptic_amount > 1) {
		haptic_amount = 1;
	}
	SDL_HapticRumblePlay(haptic, haptic_amount, 160);

	return true;
}
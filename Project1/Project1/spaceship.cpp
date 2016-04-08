#include <SDL.h>
#include <iostream>

#include "spaceship.h"
#include "renderer.h"

void Ship::render() {
	double angle = r->calculate_angle(face_dir_x, face_dir_y);

	if (invincibility_cooldown > 0) {
		if (invincibility_cooldown % invincibility_switch_rate < invincibility_switch_rate/3) {
			r->render_texture(ship_invincible_tex, x_pos, y_pos, angle, 3);
		} else {
			r->render_texture(ship_tex, x_pos, y_pos, angle, 3);
		}
	} else {
		r->render_texture(ship_tex, x_pos, y_pos, angle, 3);
	}

	// render gun
	r->render_texture_edge_spin(cannon_tex, x_pos, y_pos, r->calculate_angle(gun_dir_x, gun_dir_y), 1);

	// render laser sight
	//render_line(x_pos, y_pos, gun_dir_x, gun_dir_y);
}

void Ship::take_knockback(int dir_x, int dir_y, int base_knockback, int knockback_scaling, int damage, SDL_Haptic* haptic) {
	if (invincibility_cooldown != 0) return;

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
}
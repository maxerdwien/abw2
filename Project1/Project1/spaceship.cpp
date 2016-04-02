#include <SDL.h>
#include <iostream>

#include "spaceship.h"
#include "renderer.h"

void Ship::render() {
	double angle = r->calculate_angle(face_dir_x, face_dir_y);

	if (invincibility_cooldown > 0) {
		r->render_texture(ship_invincible_tex, x_pos / 10000, y_pos / 10000, angle, 3);
	} else {
		r->render_texture(ship_tex, x_pos / 10000, y_pos / 10000, angle, 3);
	}

	// render gun
	{
		SDL_Rect rect;

		SDL_QueryTexture(cannon_tex, NULL, NULL, &rect.w, &rect.h);
		rect.x = x_pos / 10000 - rect.w;
		rect.y = y_pos / 10000 - rect.h;
		//std::cout << rect.x << std::endl;
		SDL_Point* point = new SDL_Point;
		point->x = 2;
		point->y = 11;
		r->RenderCopyEx(cannon_tex, NULL, &rect, r->calculate_angle(gun_dir_x, gun_dir_y), point, SDL_FLIP_NONE);
		free(point);
	}

	// render laser sight
	//render_line(x_pos, y_pos, gun_dir_x, gun_dir_y);
}

void Ship::take_knockback(int dir_x, int dir_y, int base_knockback, int knockback_scaling, int damage, SDL_Haptic* haptic) {
	if (invincibility_cooldown != 0) return;

	// knockback
	int total_knockback = (int)(100.0 * (base_knockback + (((double)percent) / 100.0)*knockback_scaling) / weight);
	
	double normalized_dir_x = dir_x / sqrt(pow(dir_x, 2) + pow(dir_y, 2));
	double normalized_dir_y = dir_y / sqrt(pow(dir_x, 2) + pow(dir_y, 2));

	//std::cout << "total_knockback: " << total_knockback << "\tnorm_dir_x: " << normalized_dir_x << "\tnorm_dir_y: " << normalized_dir_y << std::endl;
	//std::cout << "x_vel_before " << x_vel << "\ty_vel_before " << y_vel << std::endl;

	x_vel += (int)(1000.0*total_knockback*normalized_dir_x);
	y_vel += (int)(1000.0*total_knockback*normalized_dir_y);

	//std::cout << "x_vel_after " << x_vel << "\ty_vel_after " << y_vel << std::endl;

	// damage
	percent += damage;
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
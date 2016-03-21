#include <SDL.h>

#include "spaceship.h"
#include "renderer.h"

void Ship::render() {
	double angle = calculate_angle(face_dir_x, face_dir_y);

	if (invincibility_cooldown > 0) {
		render_texture(ship_invincible_tex, x_pos / 10000, y_pos / 10000, angle, 3);
	} else {
		render_texture(ship_tex, x_pos / 10000, y_pos / 10000, angle, 3);
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
		RenderCopyEx(cannon_tex, NULL, &rect, calculate_angle(gun_dir_x, gun_dir_y), point, SDL_FLIP_NONE);
		free(point);
	}
}
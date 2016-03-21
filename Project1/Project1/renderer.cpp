#include <SDL_image.h>
#include <iostream>

#include "renderer.h"

double calculate_angle(int x_vel, int y_vel) {
	double angle;
	double conversion = 180 / M_PI;
	angle = atan2(y_vel, x_vel) * conversion + 90;
	return angle;
}

void render_texture(SDL_Texture* texture, int x, int y, double angle, double scaling) {
	SDL_Rect rect;

	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	rect.w *= scaling;
	rect.h *= scaling;
	rect.x = x - rect.w / 2;
	rect.y = y - rect.h / 2;

	RenderCopyEx(texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
}

int RenderCopyEx(SDL_Texture* texture,	const SDL_Rect* srcrect, const SDL_Rect* dstrect, const double angle, const SDL_Point* center, const SDL_RendererFlip flip) {
	return SDL_RenderCopyEx(renderer, texture, srcrect, dstrect, angle, center, flip);
}

SDL_Texture* LoadTexture(const char* file) {
	SDL_Texture* tex =  IMG_LoadTexture(renderer, file);
	if (!tex) {
		std::cout << "error loading texture from " << file << "\n\terror number: " << SDL_GetError() << std::endl;
	}
	return tex;
}
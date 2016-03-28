#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>

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
		std::cout << SDL_GetError() << std::endl;
	}
	return tex;
}

void render_text(int x, int y, const std::string& s) {
	// todo: don't reopen this every time
	TTF_Font* caladea36 = TTF_OpenFont("..\\Project1\\assets\\caladea-regular.ttf", 36); //this opens a font style and sets a size

	//char* cstr = new char[s.length() + 1];
	//std::strcpy(cstr, s.c_str());

	SDL_Color White = { 255, 255, 255 };
	SDL_Surface* surface = TTF_RenderText_Blended(caladea36, s.c_str(), White); //Create the sdl surface
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); //Convert to texture
	SDL_Rect rect; //create a rect
	rect.x = x;
	rect.y = y;
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

	TTF_CloseFont(caladea36);
}

void render_line(int x_start, int y_start, int x_dir, int y_dir) {
	// set color to red, cause lasers are red
	SDL_SetRenderDrawColor(renderer, 128, 0, 0, SDL_ALPHA_OPAQUE);
	/*
	int red = (std::rand()*256) / RAND_MAX;
	int green = (std::rand() * 256) / RAND_MAX;
	int blue = (std::rand() * 256) / RAND_MAX;
	SDL_SetRenderDrawColor(renderer, red, green, blue, SDL_ALPHA_OPAQUE);
	*/
	
	int x_end = x_start+x_dir*100000;
	int y_end = y_start+y_dir*100000;
	/*
	if (x_end > WINDOW_WIDTH) {
		y_end *= ((double)WINDOW_WIDTH / x_end);
		x_end = WINDOW_WIDTH;
	}
	if (y_end > WINDOW_HEIGHT) {
		x_end *= ((double)WINDOW_HEIGHT / y_end);
		y_end = WINDOW_HEIGHT;
	}
	*/
	SDL_RenderDrawLine(renderer, x_start/10000, y_start/10000, x_end/10000, y_end/10000);
}
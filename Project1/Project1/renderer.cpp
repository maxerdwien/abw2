#include <iostream>
#include <string>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "renderer.h"

Renderer::Renderer(SDL_Renderer* r, int w, int h) {
	renderer = r;

	WINDOW_WIDTH = w;
	WINDOW_HEIGHT = h;

	TTF_Init();

	TTF_Font *font;
	font = TTF_OpenFont("font.ttf", 16);
	if (!font) {
		printf("TTF_OpenFont: %s\n", TTF_GetError());
		// handle error
	}

	TTF_Font* f = TTF_OpenFont("..\\Project1\\assets\\caladea-regular.ttf", 36); // for 720p
	caladea36 = TTF_OpenFont("..\\Project1\\assets\\caladea-regular.ttf", 36); // for 720p
	caladea54 = TTF_OpenFont("..\\Project1\\assets\\caladea-regular.ttf", 54); // for 1080p
}

double Renderer::calculate_angle(int x_vel, int y_vel) {
	double angle;
	double conversion = 180 / M_PI;
	angle = atan2(y_vel, x_vel) * conversion + 90;
	return angle;
}

void Renderer::render_texture(SDL_Texture* texture, int x, int y, double angle, double scaling) {
	SDL_Rect rect;

	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	rect.w *= scaling;
	rect.h *= scaling;
	rect.x = x - rect.w / 2;
	rect.y = y - rect.h / 2;

	RenderCopyEx(texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
}

int Renderer::RenderCopyEx(SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect, const double angle, const SDL_Point* center, const SDL_RendererFlip flip) {
	return SDL_RenderCopyEx(renderer, texture, srcrect, dstrect, angle, center, flip);
}

SDL_Texture* Renderer::LoadTexture(const char* file) {
	SDL_Texture* tex =  IMG_LoadTexture(renderer, file);
	if (!tex) {
		std::cout << SDL_GetError() << std::endl;
	}
	return tex;
}

void Renderer::render_text(int x, int y, const std::string& s) {

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
}

void Renderer::render_text_centered(int x, int y, const std::string& s) {

	SDL_Color White = { 255, 255, 255 };
	SDL_Surface* surface = TTF_RenderText_Blended(caladea36, s.c_str(), White); //Create the sdl surface
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); //Convert to texture
	SDL_Rect rect; //create a rect
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	rect.x = x - rect.w/2;
	rect.y = y;
	
	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
}

void Renderer::render_line(int x_start, int y_start, int x_dir, int y_dir) {
	// set color to yellow, cause sparks are yellow
	SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
	
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

void Renderer::render_line_thick(int x_start, int y_start, int x_dir, int y_dir) {
	// set color to red, cause lasers are red
	SDL_SetRenderDrawColor(renderer, 128, 0, 0, SDL_ALPHA_OPAQUE);

	int x_end = x_start + x_dir * 100000;
	int y_end = y_start + y_dir * 100000;
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
	SDL_RenderDrawLine(renderer, x_start / 10000, y_start / 10000, x_end / 10000, y_end / 10000);
	SDL_RenderDrawLine(renderer, x_start / 10000+1, y_start / 10000, x_end / 10000+1, y_end / 10000);
	SDL_RenderDrawLine(renderer, x_start / 10000-1, y_start / 10000, x_end / 10000-1, y_end / 10000);
	SDL_RenderDrawLine(renderer, x_start / 10000, y_start / 10000+1, x_end / 10000, y_end / 10000+1);
	SDL_RenderDrawLine(renderer, x_start / 10000, y_start / 10000-1, x_end / 10000, y_end / 10000-1);
}

void Renderer::render_sparks(int x1, int y1, int x2, int y2) {
	SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawLine(renderer, x1 / 10000, y1 / 10000, x2 / 10000, y2 / 10000);
}
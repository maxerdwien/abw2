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
	ratio = (double)WINDOW_HEIGHT / 720.0;

	TTF_Init();

<<<<<<< HEAD
	const char* file = "..\\Project1\\assets\\caladea-regular.ttf";
	caladea_large = TTF_OpenFont(file, 44 * ratio);
	caladea = TTF_OpenFont(file, 36 * ratio);
	caladea_small = TTF_OpenFont(file, 28 * ratio);
=======
	const char* font_file = "..\\Project1\\assets\\caladea-regular.ttf";
	caladea_large = TTF_OpenFont(font_file, 44 * ratio); 
	caladea = TTF_OpenFont(font_file, 36 * ratio);
	caladea_small = TTF_OpenFont(font_file, 28 * ratio);

	render_normal = true;
	render_debug = false;

	hitbox_tex = LoadTexture("..\\Project1\\assets\\hitbox.png");
	hurtbox_tex = LoadTexture("..\\Project1\\assets\\hurtbox.png");
	invincible_hurtbox_tex = LoadTexture("..\\Project1\\assets\\invincible_hurtbox.png");
	activation_hitbox_tex = LoadTexture("..\\Project1\\assets\\activation_hitbox.png");
>>>>>>> origin/master
	
}

double Renderer::atan2_degrees(int x_vel, int y_vel) {
	double angle;
	double conversion = 180 / M_PI;
	angle = atan2(y_vel, x_vel) * conversion + 90;
	return angle;
}

void Renderer::render_texture(SDL_Texture* texture, int x, int y, double angle, double scaling) {
	x *= ratio / 10000;
	y *= ratio / 10000;
	scaling *= ratio;

	SDL_Rect rect;
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	rect.w *= scaling;
	rect.h *= scaling;
	rect.x = x - rect.w / 2;
	rect.y = y - rect.h / 2;

	RenderCopyEx(texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
}

void Renderer::render_texture_edge_spin(SDL_Texture* texture, int x, int y, double angle, double scaling) {
	x *= ratio / 10000;
	y *= ratio / 10000;
	scaling *= ratio;

	SDL_Rect rect;
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	rect.w *= scaling;
	rect.h *= scaling;
	rect.x = x - rect.w / 2;
	rect.y = y - rect.h;

	SDL_Point p;
	p.x = rect.w / 2;
	p.y = rect.h;

	RenderCopyEx(texture, NULL, &rect, angle, &p, SDL_FLIP_NONE);
}

void Renderer::render_texture_abs_size(SDL_Texture* texture, int x, int y, double angle, int radius) {
	x *= ratio / 10000;
	y *= ratio / 10000;
	radius *= ratio / 10000;

	SDL_Rect rect;
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	rect.w = radius*2;
	rect.h = radius*2;
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

void Renderer::render_text(int x, int y, const std::string& s, bool center_x, bool center_y, bool highlight, font_size size, int green_and_blue, int alpha) {
	x *= ratio / 10000;
	y *= ratio / 10000;

	// select font
	TTF_Font* font;
	switch (size) {
	case small_f:
		font = caladea_small;
		break;
	case medium_f:
		font = caladea;
		break;
	case large_f:
		font = caladea_large;
		break;
	default:
		font = caladea;
	}

	SDL_Color White = { 255, green_and_blue, green_and_blue };
	SDL_Surface* surface = TTF_RenderText_Blended(font, s.c_str(), White);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect rect;
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	if (center_x) {
		rect.x = x - rect.w / 2;
	} else {
		rect.x = x;
	}
	if (center_y) {
		rect.y = y - rect.h / 2;
	} else {
		rect.y = y;
	}

	if (highlight) {
		SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);

		SDL_RenderFillRect(renderer, &rect);
	}
	SDL_SetTextureAlphaMod(texture, alpha);
	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
}

void Renderer::render_line_w_end(int x_start, int y_start, int x_end, int y_end) {
	x_start *= ratio / 10000;
	y_start *= ratio / 10000;
	x_end *= ratio / 10000;
	y_end *= ratio / 10000;

	SDL_RenderDrawLine(renderer, x_start, y_start, x_end, y_end);
}

void Renderer::render_line(int x_start, int y_start, int x_dir, int y_dir) {
	x_start *= ratio / 10000;
	y_start *= ratio / 10000;
	
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
	SDL_RenderDrawLine(renderer, x_start, y_start, x_end, y_end);
}

void Renderer::render_line_thick(int x_start, int y_start, int x_dir, int y_dir) {
	x_start *= ratio / 10000;
	y_start *= ratio / 10000;

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
	SDL_RenderDrawLine(renderer, x_start, y_start, x_end, y_end);

	SDL_RenderDrawLine(renderer, x_start+1, y_start, x_end+1, y_end);
	SDL_RenderDrawLine(renderer, x_start-1, y_start, x_end-1, y_end);
	SDL_RenderDrawLine(renderer, x_start, y_start+1, x_end, y_end+1);
	SDL_RenderDrawLine(renderer, x_start, y_start-1, x_end, y_end-1);

	SDL_RenderDrawLine(renderer, x_start + 1, y_start + 1, x_end + 1, y_end + 1);
	SDL_RenderDrawLine(renderer, x_start + 1, y_start - 1, x_end + 1, y_end - 1);
	SDL_RenderDrawLine(renderer, x_start - 1, y_start + 1, x_end - 1, y_end + 1);
	SDL_RenderDrawLine(renderer, x_start - 1, y_start - 1, x_end - 1, y_end - 1);

	SDL_RenderDrawLine(renderer, x_start + 2, y_start, x_end + 2, y_end);
	SDL_RenderDrawLine(renderer, x_start - 2, y_start, x_end - 2, y_end);
	SDL_RenderDrawLine(renderer, x_start, y_start + 2, x_end, y_end + 2);
	SDL_RenderDrawLine(renderer, x_start, y_start - 2, x_end, y_end - 2);
}


void Renderer::render_solid_bg() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_Rect s;
	s.x = 0;
	s.y = 0;
	s.w = WINDOW_WIDTH;
	s.h = WINDOW_HEIGHT;
	SDL_RenderFillRect(renderer, &s);
}



void Renderer::render_cross_bars(int width) {
	width /= 10000;
	width *= ratio;

	// render character select dividers vertical
	{
		SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
		SDL_Rect s;
		s.x = WINDOW_WIDTH / 2 - width / 2;
		s.y = 0;
		s.w = width;
		s.h = WINDOW_HEIGHT;
		SDL_RenderFillRect(renderer, &s);
	}

	// render character select dividers horizontal
	{
		SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
		SDL_Rect s;
		s.x = 0;
		s.y = WINDOW_HEIGHT / 2 - width / 2;
		s.w = WINDOW_WIDTH;
		s.h = width;
		SDL_RenderFillRect(renderer, &s);
	}
}

void Renderer::render_rect(int x, int y, int w, int h) {
	x *= ratio / 10000;
	y *= ratio / 10000;
	w *= ratio / 10000;
	h *= ratio / 10000;

	SDL_Rect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	SDL_RenderFillRect(renderer, &r);
}

int Renderer::SetRenderDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return SDL_SetRenderDrawColor(renderer, r, g, b, a);
}
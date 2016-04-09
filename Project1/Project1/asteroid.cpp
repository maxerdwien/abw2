#include <string>
#include <SDL_image.h>

#include "renderer.h"
#include "asteroid.h"

Asteroid::Asteroid(int x, int y, Renderer* rend) {
	x_pos = x;
	y_pos = y;

	degrees_per_frame = (double)(rand() % 180)/60.0 - (double)90/60;

	r = rend;

	tex = r->LoadTexture("..\\Project1\\assets\\sun.png");
}

void Asteroid::render() {
	r->render_texture_abs_size(tex, x_pos, y_pos, angle, radius);
}

void Asteroid::update() {
	angle += degrees_per_frame;
}
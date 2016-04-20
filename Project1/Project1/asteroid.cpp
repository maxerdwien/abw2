#include <string>
#include <SDL_image.h>

#include "renderer.h"
#include "asteroid.h"

Asteroid::Asteroid(int x, int y, int x_v, int y_v, Renderer* rend) {
	x_pos = x;
	y_pos = y;

	x_vel = x_v;
	y_vel = y_v;

	degrees_per_frame = (double)(rand() % 180) / 60.0 - 90.0 / 60.0;

	r = rend;

	tex = r->LoadTexture("..\\Project1\\assets\\asteroid1.png");
}

void Asteroid::render() {
	r->render_texture_abs_size(tex, x_pos, y_pos, angle, radius);
}

void Asteroid::update() {
	angle += degrees_per_frame;

	x_pos += x_vel;
	y_pos += y_vel;
}
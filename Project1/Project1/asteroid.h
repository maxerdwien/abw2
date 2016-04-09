#pragma once

class Asteroid {
private:
	class Renderer* r;
	SDL_Texture* tex;

	double angle;
	double degrees_per_frame;
public:
	int x_pos;
	int y_pos;

	int x_vel = 0;
	int y_vel = 0;

	int radius = 10000 * 100;

	Asteroid(int x, int y, Renderer* rend);

	void render();
	void update();
};
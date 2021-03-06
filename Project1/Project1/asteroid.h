#pragma once

class Asteroid {
private:
	class Renderer* r;
	SDL_Texture* tex = NULL;

	double angle;
	double degrees_per_frame;
public:
	int x_pos;
	int y_pos;

	int x_vel = 0;
	int y_vel = 0;

	int radius = 10000 * 100;

	int tex_number;

	Asteroid(Renderer* rend);
	Asteroid(int x, int y, int x_v, int y_v, Renderer* rend);
	~Asteroid();

	void get_resources();

	void render();
	void update();

	int serialize(char* buf, int i);
	int deserialize(char* buf, int i);
};
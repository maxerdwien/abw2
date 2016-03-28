#pragma once

class Polar : public Ship {
private:
	SDL_Texture* bullet_tex;

public:
	struct bullet* bullets[1000];
	int num_bullets = 0;

	const int spread_delay = 30;
	int spread_cooldown = 0;

	Polar(int identifier, int x, int y);

	void update();

	void fire_1();
	void update_projectiles_1(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]);
	void render_projectiles_1();

	void fire_2();
	void update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]);
	void render_projectiles_2();

	void fire_3();
	void update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]);
	void render_projectiles_3();
};
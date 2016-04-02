#pragma once

const int num_flame_hitboxes = 2;
const int flame_radii[num_flame_hitboxes] = { 10000*20, 10000*18 };
const int flame_dists[num_flame_hitboxes] = { 10000*88, 10000*80 };

class Black : public Ship {
private:
	SDL_Texture* bullet_tex;
	SDL_Texture* flame_tex_1;
	SDL_Texture* flame_tex_2;

public:
	struct bullet* bullets[1000];
	int num_bullets = 0;

	const int burst_delay_1 = 120;
	int burst_cooldown_1 = 0;

	const int burst_delay_2 = 5;
	int burst_cooldown_2 = 5;

	const int burst_shot_number = 5;
	int burst_shot_current = 0;

	// in frames
	int charge_shot_charge = 0;

	bool flame_active;
	const int flame_switch_delay = 5;
	int flame_switch_cooldown = 0;
	int current_flame = 0;

	Black(int identifier, int x, int y, Renderer* rend);

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
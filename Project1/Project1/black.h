#pragma once

const int num_flame_hitboxes = 5;
const int flame_radii[num_flame_hitboxes] = { 10000*33, 10000*33, 10000*25, 10000*20, 10000*14 };
const int flame_dists[num_flame_hitboxes] = { 10000*195, 10000*160, 10000*110, 10000*85, 10000* 60};

class Black : public Ship {
private:
	SDL_Texture* bullet_tex;
	SDL_Texture* flame_tex_1;
	SDL_Texture* flame_tex_2;
	SDL_Texture* hitbox_tex;

	SDL_Texture* bounce_bullet_tex;

	Mix_Chunk* bullet_sfx;
	Mix_Chunk* flamethrower_sfx;
	Mix_Chunk* charging_shot_sfx;

	int charging_channel;
	int flamethrower_channel;

	struct bullet* bullets[1000];
	int num_bullets = 0;

	const int burst_delay_1 = 60;
	int burst_cooldown_1 = 0;

	const int burst_delay_2 = 4;
	int burst_cooldown_2 = 4;

	const int burst_shot_number = 6;
	int burst_shot_current = 0;

	// in frames
	int charge_shot_charge = 0;

	bool flame_active;
	const int flame_switch_delay = 5;
	int flame_switch_cooldown = 0;
	int current_flame = 0;

public:
	Black(int identifier, int a1, int a2, int x, int y, Renderer* rend);
	~Black();

	void update();
	void die();

	void fire_1();
	void update_projectiles_1(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]);
	void render_projectiles_1();

	void fire_2();
	void update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]);
	void render_projectiles_2();

	void fire_3();
	void update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]);
	void render_projectiles_3();
};
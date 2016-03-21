#pragma once

class Black : public Ship {
private:
	SDL_Texture* bullet_tex;
	SDL_Texture* missile_tex;
	SDL_Texture* explosion_tex;

public:
	struct bullet* bullets[1000];
	int num_bullets = 0;

	struct missile* missiles[100];
	int num_missiles = 0;

	const int burst_delay_1 = 120;
	int burst_cooldown_1 = 0;

	const int burst_delay_2 = 5;
	int burst_cooldown_2 = 5;

	const int burst_shot_number = 5;
	int burst_shot_current = 0;

	// in frames
	int charge_shot_charge = 0;

	Black(int identifier, int x, int y);

	void update();

	void fire_1();
	void update_projectiles_1(int min_x, int max_x, int min_y, int max_y, int num_players, Ship* ships[]);
	void render_projectiles_1();

	void fire_2();
	void update_projectiles_2(int min_x, int max_x, int min_y, int max_y, int num_players, Ship* ships[]);
	void render_projectiles_2();
};
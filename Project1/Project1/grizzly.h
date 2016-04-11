#pragma once

class Grizzly : public Ship {
private:
	SDL_Texture* bullet_tex;
	SDL_Texture* missile_tex;
	SDL_Texture* mine_tex;
	SDL_Texture* explosion_tex;

	SDL_Texture* bounce_bullet_tex;
	SDL_Texture* bounce_missile_tex;

	Mix_Chunk* bullet_sfx;
	Mix_Chunk* missile_launch_sfx;
	Mix_Chunk* explosion_sfx;
	Mix_Chunk* mine_deploy_sfx;

	struct bullet* bullets[1000];
	int num_bullets = 0;

	struct missile* missiles[100];
	int num_missiles = 0;

	struct missile* mines[100];
	int num_mines = 0;

	const int cannon_delay = 10;
	int cannon_cooldown = 0;

	const int missile_delay = 50;
	int missile_cooldown = 0;

	const int mine_delay = 110;
	int mine_cooldown = 0;

public:
	

	Grizzly(int identifier, int x, int y, Renderer* rend);
	~Grizzly();

	void update();
	//void render();

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
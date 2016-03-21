#pragma once

const int SPACESHIP_MAX_ACCEL = 7000;
const int SPACESHIP_MAX_FRICTION= 90;
const int SPACESHIP_STEADY_FRICTION = 200000000;

const int SPACESHIP_MIN_GUN_DIR = 25000;

const int SPACESHIP_MAX_PERCENT = 999;

const int SPACESHIP_STAMINA_PER_FRAME = 5;

class Ship {
	public:
		// units of one ten thousandth of a pixel
		int x_pos = 0;
		int y_pos = 0;

		int x_vel = 0;
		int y_vel = 0;

		int x_accel = 0;
		int y_accel = 0;

		int move_dir_x = 0;
		int move_dir_y = 0;

		int face_dir_x = 0;
		int face_dir_y = 0;

		int gun_dir_x = 1;
		int gun_dir_y = 0;

		bool do_fire_1 = false;
		bool fire_burst = false;
		bool fire_spread = false;
		bool do_fire_2 = false;

		int percent = 0;

		int lives = 4;

		const int respawn_invincibility_delay = 120;
		int invincibility_cooldown = 0;

		int stamina = 0;
		int stamina_max = 0;

		int radius;
		int weight;

		int id;

		virtual void update() = 0;

		virtual void fire_1() = 0;
		virtual void update_projectiles_1(int min_x, int max_x, int min_y, int max_y, int num_players, Ship* ships[]) = 0;
		virtual void render_projectiles_1() = 0;

		virtual void fire_2() = 0;
		virtual void update_projectiles_2(int min_x, int max_x, int min_y, int max_y, int num_players, Ship* ships[]) = 0;
		virtual void render_projectiles_2() = 0;
};



class Grizzly : public Ship {
private:
	SDL_Texture* bullet_tex;
	SDL_Texture* missile_tex;
	SDL_Texture* explosion_tex;

public:
	struct bullet* bullets[1000];
	int num_bullets = 0;

	struct missile* missiles[100];
	int num_missiles = 0;

	const int cannon_delay = 10;
	int cannon_cooldown = 0;

	const int burst_delay_1 = 120;
	int burst_cooldown_1 = 0;

	const int burst_delay_2 = 5;
	int burst_cooldown_2 = 5;

	const int burst_shot_number = 5;
	int burst_shot_current = 0;

	const int spread_delay = 25;
	int spread_cooldown = 0;

	const int missile_delay = 50;
	int missile_cooldown = 0;

	Grizzly(int identifier, int x, int y);

	void update();

	void fire_1();
	void update_projectiles_1(int min_x, int max_x, int min_y, int max_y, int num_players, Ship* ships[]);
	void render_projectiles_1();

	void fire_2();
	void update_projectiles_2(int min_x, int max_x, int min_y, int max_y, int num_players, Ship* ships[]);
	void render_projectiles_2();
};
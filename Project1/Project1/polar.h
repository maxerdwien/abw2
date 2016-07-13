#pragma once

// in radians per frame
const double polar_gun_turn_speed = M_PI / 30;


class Polar : public Ship {
private:
	SDL_Texture* bullet_tex;
	SDL_Texture* missile_tex;
	SDL_Texture* vortex_tex;

	SDL_Texture* bounce_bullet_tex;
	SDL_Texture* bounce_missile_tex;

	Mix_Chunk* bullet_sfx;
	Mix_Chunk* laser_sfx;
	Mix_Chunk* missile_launch_sfx;
	Mix_Chunk* blackhole_sfx;

	int laser_channel;

	class Bullet* bullets[1000];
	int num_bullets = 0;

	const int spread_delay = 30;
	int spread_cooldown = 0;

	class Gravity_Missile* g_missiles[10];
	int num_g_missiles = 0;

	bool missile_click_used = false;

	const int missile_delay = 160;
	int missile_cooldown = 0;

	bool laser_active = false;

	int laser_start_x;
	int laser_start_y;
	int laser_end_x;
	int laser_end_y;

	class Spark* sparks[1000];
	int num_sparks = 0;

public:
	

	Polar(int identifier, int a1, int a2, int x, int y, Renderer* rend);
	~Polar();

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

	double Polar::get_dist(long x_1, long y_1, long x_2, long y_2, long x_0, long y_0);

	int serialize(char* buf, int i);
	int deserialize(char*buf, int i);
};
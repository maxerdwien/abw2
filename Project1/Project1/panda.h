#pragma once

enum class squiggle_state {
	inactive,
	shooting,
	growing,
	shrinking,

	MAX
};

class Panda : public Ship {
private:
	SDL_Texture* bullet_tex;
	SDL_Texture* missile_tex;
	SDL_Texture* squiggle_tex;
	SDL_Texture* squiggle_circle_tex;
	SDL_Texture* squiggle_rect_tex;

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

	int frames_charging_spread;
	//int mod;
	double total_spread_angle; // todo: remove?


	bool laser_active = false;
	const int laser_damage_delay = 2;
	int laser_damage_cooldown = 0;

	int laser_start_x;
	int laser_start_y;
	int laser_end_x;
	int laser_end_y;

	class Spark* sparks[1000];
	int num_sparks = 0;

	squiggle_state squiggle_state = squiggle_state::inactive;
	int squiggle_x;
	int squiggle_y;

	double squiggle_angle;
	const int squiggle_dist_per_frame = 150000;
	const int squiggle_segs_per_frame = 3;

	// one hundred seconds worth of segments. way too many.
	class Squiggle_Segment* squiggle_segments[6000];
	int num_squiggle_segments = 0;

	const int normal_squiggle_width = 5 * 10000;
	const int max_squiggle_width = 50 * 10000;
	const int squiggle_width_per_frame = 15000;
	const int squiggle_shrinkage_rate = 8000;
	int squiggle_width = normal_squiggle_width;

	int squiggle_ship_hits_cooldown[4];

public:
	Panda(int identifier, int a1, int a2, int x, int y, Renderer* rend);
	~Panda();

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

	double Panda::get_dist(long x_1, long y_1, long x_2, long y_2, long x_0, long y_0);

	int serialize(char* buf, int i);
	int deserialize(char*buf, int i);
};
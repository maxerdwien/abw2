#pragma once

const int SPACESHIP_MIN_GUN_DIR = 25000;

const int SPACESHIP_MAX_PERCENT = 999;

class Ship {
	protected:
		class Renderer* r;
		SDL_Texture* ship_tex;
		SDL_Texture* ship_invincible_tex;
		SDL_Texture* cannon_tex;
		SDL_Texture* shield_tex;
		SDL_Texture* bounce_tex;
		SDL_Texture* thrust_low_tex;
		SDL_Texture* thrust_medium_tex;
		SDL_Texture* thrust_high_tex;

	public:
		// units of one ten thousandth of a pixel
		int x_pos = 0;
		int y_pos = 0;

		int x_vel = 0;
		int y_vel = 0;

		int x_accel = 0;
		int y_accel = 0;

		long max_accel;
		long friction_limiter = 2000000;
		unsigned long constant_friction = 1500000000;

		int move_dir_x = 0;
		int move_dir_y = 0;

		int face_dir_x = 0;
		int face_dir_y = 0;

		int left_stick_x = 1;
		int left_stick_y = 0;

		int desired_gun_dir_x = 1;
		int desired_gun_dir_y = 0;

		int gun_dir_x = 1;
		int gun_dir_y = 0;

		int right_stick_x = 1;
		int right_stick_y = 0;

		int respawn_x;
		int respawn_y;

		bool do_fire_1 = false;
		bool do_fire_2 = false;
		bool do_fire_3 = false;

		bool do_speed_boost = false;

		const int speed_boost_delay = 120;
		int speed_boost_cooldown = 0;

		const int speed_boost_high_engine_time = 30;

		int percent = 0;

		int lives = 5;

		const int respawn_invincibility_delay = 180;
		int invincibility_cooldown = 90;
		const int invincibility_switch_rate = 20;

		int stamina = 0;
		int stamina_max = 0;
		int stamina_per_frame;

		int normal_radius;
		int radius;
		int weight;

		int normal_gun_length = 10000 * 21;
		int gun_length = 10000 * 21;

		// todo: make this use the constant NUM_ITEMS
		int item_times[6];

		int last_hit = -1;

		int kills[100];
		int num_kills = 0;
		int num_suicides = 0;

		int damage_done = 0;
		int damage_taken = 0;

		int place = -1;

		int id;
		int ally1;
		int ally2;

		virtual ~Ship();

		virtual void update() = 0; // todo: rename?
		//virtual void update_from_input
		virtual void die() = 0;
		void render();

		bool take_knockback(int dir_x, int dir_y, int base_knockback, int knockback_scaling, int damage, SDL_Haptic* haptic);

		virtual void fire_1() = 0;
		virtual void update_projectiles_1(int min_x, int max_x, int min_y, int max_y, Ship* ships[], class Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) = 0;
		virtual void render_projectiles_1() = 0;

		virtual void fire_2() = 0;
		virtual void update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) = 0;
		virtual void render_projectiles_2() = 0;

		virtual void fire_3() = 0;
		virtual void update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]) = 0;
		virtual void render_projectiles_3() = 0;

		int serialize_ship(char* buf, int i);
		int deserialize_ship(char* buf, int i);

		virtual int serialize(char* buf, int i) = 0;
		virtual int deserialize(char*buf, int i) = 0;
};




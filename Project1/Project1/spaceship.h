#pragma once

const int SPACESHIP_MIN_GUN_DIR = 25000;

const int SPACESHIP_MAX_PERCENT = 999;

class Ship {
	protected:
		SDL_Texture* ship_tex;
		SDL_Texture* ship_invincible_tex;
		SDL_Texture* cannon_tex;

	public:
		// units of one ten thousandth of a pixel
		int x_pos = 0;
		int y_pos = 0;

		int x_vel = 0;
		int y_vel = 0;

		int x_accel = 0;
		int y_accel = 0;

		int max_accel;
		int friction_limiter;
		int constant_friction;

		const int MAX_FRICTION_ACCEL = 1000000;

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

		const int speed_boost_delay = 60;
		int speed_boost_cooldown = 0;

		int percent = 0;

		int lives = 4;

		const int respawn_invincibility_delay = 120;
		int invincibility_cooldown = 0;

		int stamina = 0;
		int stamina_max = 0;
		int stamina_per_frame;

		int radius;
		int weight;

		int id;

		const int GUN_LENGTH = 12 * 10000;

		virtual void update() = 0;
		void render();

		void take_knockback(int dir_x, int dir_y, int base_knockback, int knockback_scaling, int damage, SDL_Haptic* haptic);

		virtual void fire_1() = 0;
		virtual void update_projectiles_1(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) = 0;
		virtual void render_projectiles_1() = 0;

		virtual void fire_2() = 0;
		virtual void update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) = 0;
		virtual void render_projectiles_2() = 0;

		virtual void fire_3() = 0;
		virtual void update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], SDL_Haptic* haptics[]) = 0;
		virtual void render_projectiles_3() = 0;
};




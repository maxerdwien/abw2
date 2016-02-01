enum Control {
	manual,
	stabilize,
	orbit
}; 

struct spaceship {
	// units of one ten thousandth of a pixel
	int x_pos = 0;
	int y_pos = 0;

	int x_vel = 0;
	int y_vel = 0;

	int x_accel = 0;
	int y_accel = 0;

	int move_dir_x = 0;
	int move_dir_y = 0;

	int gun_dir_x = 1;
	int gun_dir_y = 0;

	const int MIN_GUN_DIR = 20000;

	const int cannon_delay = 200;

	int cannon_cooldown = 0;

	int x_vel_accumulator = 0;
	int y_vel_accumulator = 0;

	Control control_state = manual;
};

const int SPACESHIP_MAX_ACCEL = 3000;

struct spaceship* init_spaceship();
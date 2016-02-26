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

	const int burst_delay_1 = 2000;
	int burst_cooldown_1 = 0;

	const int burst_delay_2 = 80;
	int burst_cooldown_2 = 0;

	const int burst_shot_number = 5;
	int burst_shot_current = 0;

	int x_vel_accumulator = 0;
	int y_vel_accumulator = 0;

	bool fire_normal = false;
	bool fire_burst = false;

	struct bullet* bullets[1000];
	int num_bullets = 0;

	const int stamina_max = 1000;
	const int stamina_per_s = 200;
	int stamina;
};

const int SPACESHIP_MAX_ACCEL = 3000;
const int SPACESHIP_MAX_VEL = 100;

struct spaceship* init_spaceship(int x, int y);
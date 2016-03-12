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

	bool fire_normal = false;
	bool fire_burst = false;
	bool fire_spread = false;
	bool fire_missile = false;

	struct bullet* bullets[1000];
	int num_bullets = 0;

	struct missile* missiles[100];
	int num_missiles = 0;

	const int stamina_max = 1000;
	int stamina = 1000;

	int percent = 0;

	int weight;

	int lives = 4;

	const int radius = 40; // 15

	const int respawn_invincibility_delay = 120;

	int invincibility_cooldown = 0;
};

const int SPACESHIP_MAX_ACCEL = 7000;
const int SPACESHIP_MAX_FRICTION= 90;
const int SPACESHIP_STEADY_FRICTION = 200000000;

const int SPACESHIP_MAX_PERCENT = 999;

const int SPACESHIP_STAMINA_PER_FRAME = 5;

struct spaceship* init_spaceship(int x, int y, int weight);
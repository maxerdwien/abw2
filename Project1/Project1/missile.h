struct missile {
	int x_pos;
	int y_pos;

	int x_vel;
	int y_vel;

	int x_accel;
	int y_accel;

	int damage;
	int base_knockback;
	int knockback_scaling;

	bool exploded = false;

	int radius = 5 * 10000;

	bool players_hit[4];
};

const int MISSILE_ACTIVATION_RADIUS = 100 * 10000;
const int MISSILE_MAX_RADIUS = 120 * 10000;
const int MISSILE_RADIUS_PER_FRAME = 5 * 10000;

const int MISSILE_ACCEL = 1500;

struct missile* init_missile(int x_pos, int y_pos, int x_vel, int y_vel, int damage, int base_knockback, int knockback_scaling);

missile** spawn_missiles(int gun_dir_x, int gun_dir_y, int x_pos, int y_pos, int velocity, int spread, int damage, int base_knockback, int knockback_scaling);
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

	int radius = 5;

	bool players_hit[4];
};

const int MISSILE_ACTIVATION_RADIUS = 100;
const int MISSILE_MAX_RADIUS = 120;
const int MISSILE_RADIUS_PER_FRAME = 5;

const int MISSILE_ACCEL = 1500;

struct missile* init_missile(int x_pos, int y_pos, int x_vel, int y_vel, int damage, int base_knockback, int knockback_scaling);
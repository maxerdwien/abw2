struct bullet {
	// units of one ten thousandth of a pixel
	int x_pos = 0;
	int y_pos = 0;

	int x_vel = 0;
	int y_vel = 0;

	int x_accel = 0;
	int y_accel = 0;

	int damage = 1;
	int base_knockback = 0;
	int knockback_scaling = 0;

	const int radius = 5;
};

struct bullet* init_bullet(int x_pos, int y_pos, int x_vel, int y_vel, int damage, int base_knockback, int knockback_scaling);
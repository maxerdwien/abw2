struct bullet {
	// units of one ten thousandth of a pixel
	int x_pos = 0;
	int y_pos = 0;

	int x_vel = 0;
	int y_vel = 0;

	int x_accel = 0;
	int y_accel = 0;
};

struct bullet* init_bullet(int x_pos, int y_pos, int x_vel, int y_vel);
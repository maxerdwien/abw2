struct baddie {
	// units of one ten thousandth of a pixel
	int x_pos = 0;
	int y_pos = 0;

	int x_vel = 0;
	int y_vel = 0;
};

struct baddie* init_baddie(int x_pos, int y_pos, int x_vel, int y_vel);

const float BADDIE_MAX_VEL = 5000;
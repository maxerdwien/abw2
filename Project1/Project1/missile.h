struct missile {
	int x_pos;
	int y_pos;

	int x_vel;
	int y_vel;

	int damage;
	int base_knockback;
	int knockback_scaling;

	bool exploded = false;
};

struct missile* init_missile(int x_pos, int y_pos, int x_vel, int y_vel, int damage, int base_knockback, int knockback_scaling);
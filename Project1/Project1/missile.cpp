#include "missile.h"

struct missile* init_missile(int x_pos, int y_pos, int x_vel, int y_vel, int damage, int base_knockback, int knockback_scaling) {
	struct missile* miss = new struct missile;

	miss->x_pos = x_pos;
	miss->y_pos = y_pos;

	miss->x_vel = x_vel;
	miss->y_vel = y_vel;

	//miss->x_accel = x_vel / 10.0;
	//miss->y_accel = y_vel / 10.0;

	miss->damage = damage;
	miss->base_knockback = base_knockback;
	miss->knockback_scaling = knockback_scaling;

	miss->players_hit[0] = false;
	miss->players_hit[1] = false;
	miss->players_hit[2] = false;
	miss->players_hit[3] = false;

	return miss;
}


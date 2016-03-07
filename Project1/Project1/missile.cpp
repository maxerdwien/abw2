#include "missile.h"

struct missile* init_missile(int x_pos, int y_pos, int x_vel, int y_vel, int damage, int base_knockback, int knockback_scaling) {
	struct missile* miss = new struct missile;

	miss->x_pos = x_pos;
	miss->y_pos = y_pos;

	miss->x_vel = x_vel;
	miss->y_vel = y_vel;

	miss->damage = damage;
	miss->base_knockback = base_knockback;
	miss->knockback_scaling = knockback_scaling;

	return miss;
}
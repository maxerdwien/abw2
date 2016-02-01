#include "bullet.h"

struct bullet* init_bullet(int x_pos, int y_pos, int x_vel, int y_vel) {
	struct bullet* bull = new struct bullet;
	
	bull->x_pos = x_pos;
	bull->y_pos = y_pos;

	bull->x_vel = x_vel;
	bull->y_vel = y_vel;

	bull->x_accel = 0;
	bull->y_accel = 0;

	return bull;
}
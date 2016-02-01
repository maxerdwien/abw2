#include "spaceship.h"

struct spaceship* init_spaceship() {
	struct spaceship* ship = new struct spaceship;
	/*
	ship->x_pos = 0;
	ship->y_pos = 0;
	
	ship->x_vel = 0;
	ship->y_vel = 0;

	ship->x_accel = 0;
	ship->y_accel = 0;
	*/

	return ship;
}
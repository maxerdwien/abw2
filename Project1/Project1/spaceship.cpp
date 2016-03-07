#include "spaceship.h"

struct spaceship* init_spaceship(int x, int y, int weight) {
	struct spaceship* ship = new struct spaceship;

	ship->x_pos = x;
	ship->y_pos = y;

	ship->weight = weight;

	ship->fire_missile = false;
	return ship;
}
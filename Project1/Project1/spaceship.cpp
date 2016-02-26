#include "spaceship.h"

struct spaceship* init_spaceship(int x, int y) {
	struct spaceship* ship = new struct spaceship;

	ship->x_pos = x;
	ship->y_pos = y;

	ship->burst_cooldown_2 = 80;

	ship->stamina = ship->stamina_max;

	return ship;
}
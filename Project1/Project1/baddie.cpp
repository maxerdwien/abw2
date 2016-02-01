#include "baddie.h"

struct baddie* init_baddie(int x_pos, int y_pos, int x_vel, int y_vel) {
	struct baddie* bad = new struct baddie;

	bad->x_pos = x_pos;
	bad->y_pos = y_pos;

	bad->x_vel = x_vel;
	bad->y_vel = y_vel;

	return bad;
}
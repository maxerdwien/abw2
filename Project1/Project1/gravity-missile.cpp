#include "gravity-missile.h"

Gravity_Missile::Gravity_Missile(int x, int y, int x_v, int y_v) {
	x_pos = x;
	y_pos = y;
	x_vel = x_v;
	y_vel = y_v;
	damage = 2;
}
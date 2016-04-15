#pragma once

const int G_MISSILE_ACTIVATION_RADIUS = 200 * 10000;
const int G_MISSILE_MAX_RADIUS = 140 * 10000;
const int G_MISSILE_RADIUS_PER_FRAME = 1.2 * 10000;
// in degrees per frame
const int vortex_turn_rate = 10;

class Gravity_Missile {
public:
	int x_pos;
	int y_pos;

	int x_vel;
	int y_vel;

	int x_accel;
	int y_accel;

	int damage;
	int base_knockback = 1;
	int knockback_scaling = 1;

	bool exploded = false;

	int radius = 5 * 10000;

	double vortex_angle = 0;

	int damage_counter = 0;

	Gravity_Missile(int x, int y, int x_v, int y_v);
};
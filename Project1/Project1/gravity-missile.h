#pragma once

class Gravity_Missile {
public:
	int x_pos;
	int y_pos;

	int x_vel;
	int y_vel;

	int x_accel;
	int y_accel;

	int damage;
	int base_knockback;
	int knockback_scaling;

	bool exploded = false;

	const int radius = 5;

	Gravity_Missile();
};
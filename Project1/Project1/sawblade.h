#pragma once

// A thrown, spinning blade that flies out, ricochets off walls and asteroids,
// then homes back to the ship that threw it. Used by Kodiak.
class Sawblade {
public:
	// units of one ten thousandth of a pixel
	int x_pos = 0;
	int y_pos = 0;

	int x_vel = 0;
	int y_vel = 0;

	// visual rotation, in degrees
	double spin = 0;

	bool returning = false;
	int frames_alive = 0;

	int radius = 14 * 10000;

	int damage = 3;
	int base_knockback = 18;
	int knockback_scaling = 55;

	// per-target re-hit delay so a blade passing through a ship doesn't hit it every frame
	int hit_cooldowns[4];

	Sawblade(int x, int y, int x_v, int y_v);

	int serialize(char* buf, int i);
	int deserialize(char* buf, int i);
};

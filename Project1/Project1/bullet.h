#pragma once

class Bullet {
public:
	// units of one ten thousandth of a pixel
	int x_pos = 0;
	int y_pos = 0;

	int x_vel = 0;
	int y_vel = 0;

	int x_accel = 0;
	int y_accel = 0;

	int damage = 1;
	int base_knockback = 0;
	int knockback_scaling = 0;

	int radius = 5 * 10000;

	int serialize(char* buf, int i);
	int deserialize(char* buf, int i);
};

Bullet* init_bullet(int x_pos, int y_pos, int x_vel, int y_vel, int damage, int base_knockback, int knockback_scaling);

// todo: simplify this function (for missiles too)
Bullet* spawn_bullet(int gun_dir_x, int gun_dir_y, int x_pos, int y_pos, int velocity, int damage, int base_knockback, int knockback_scaling);

Bullet** spawn_spread_bullets(int gun_dir_x, int gun_dir_y, int x_pos, int y_pos, int velocity, int spread, double spread_angle, int damage, int base_knockback, int knockback_scaling);
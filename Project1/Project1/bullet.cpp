#include <math.h>
#include <string>
#include "bullet.h"

Bullet* init_bullet(int x_pos, int y_pos, int x_vel, int y_vel, int damage, int base_knockback, int knockback_scaling) {
	Bullet* bull = new Bullet;
	
	bull->x_pos = x_pos;
	bull->y_pos = y_pos;

	bull->x_vel = x_vel;
	bull->y_vel = y_vel;

	bull->damage = damage;
	bull->base_knockback = base_knockback;
	bull->knockback_scaling = knockback_scaling;

	return bull;
}

Bullet** spawn_bullets(int gun_dir_x, int gun_dir_y, int x_pos, int y_pos, int velocity, int spread, int damage, int base_knockback, int knockback_scaling) {
	double spread_angle = 3.14159 * 0.1;

	Bullet** new_bullets = (Bullet**)malloc(sizeof(Bullet*) * spread);

	if (spread % 2 == 0) {

	} else {
		double straight_theta = atan2(gun_dir_y, gun_dir_x);

		for (int i = 0; i < spread; i++) {
			double theta;
			if (i % 2 == 0) {
				theta = straight_theta + (i / 2)*spread_angle;
			} else {
				theta = straight_theta - ((i + 1) / 2)*spread_angle;
			}

			int x_vel = (int)(velocity*cos(theta));
			int y_vel = (int)(velocity*sin(theta));

			if (x_vel != 0 || y_vel != 0) {
				new_bullets[i] = init_bullet(x_pos, y_pos, x_vel, y_vel, damage, base_knockback, knockback_scaling);
			}
		}
	}

	return new_bullets;

}
#include <math.h>
#include <string>

#include "serializer.h"
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

Bullet* spawn_bullet(int gun_dir_x, int gun_dir_y, int x_pos, int y_pos, int velocity, int damage, int base_knockback, int knockback_scaling) {
	double theta = atan2(gun_dir_y, gun_dir_x);

	int x_vel = (int)(velocity*cos(theta));
	int y_vel = (int)(velocity*sin(theta));
	return init_bullet(x_pos, y_pos, x_vel, y_vel, damage, base_knockback, knockback_scaling);
}

Bullet** spawn_spread_bullets(int gun_dir_x, int gun_dir_y, int x_pos, int y_pos, int velocity, int spread, double spread_angle, int damage, int base_knockback, int knockback_scaling) {
	Bullet** new_bullets = (Bullet**)malloc(sizeof(Bullet*) * spread);

	double straight_theta = atan2(gun_dir_y, gun_dir_x);
	if (spread % 2 == 0) {
		for (int i = 0; i < spread; i++) {
			double theta;
			if (i % 2 == 0) {
				theta = straight_theta + ((i / 2)*spread_angle + spread_angle/2);
			} else {
				theta = straight_theta - ((i / 2)*spread_angle + spread_angle/2);
			}

			int x_vel = (int)(velocity*cos(theta));
			int y_vel = (int)(velocity*sin(theta));

			if (x_vel != 0 || y_vel != 0) {
				new_bullets[i] = init_bullet(x_pos, y_pos, x_vel, y_vel, damage, base_knockback, knockback_scaling);
			} else {
				throw;
			}
		}
	} else {
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
			} else {
				throw;
			}
		}
	}

	return new_bullets;
}

int Bullet::serialize(char* buf, int i) {
	i = serialize_int(x_pos, buf, i);
	i = serialize_int(y_pos, buf, i);

	i = serialize_int(x_vel, buf, i);
	i = serialize_int(y_vel, buf, i);

	i = serialize_int(radius, buf, i);

	return i;
}

int Bullet::deserialize(char* buf, int i) {
	i = deserialize_int(&x_pos, buf, i);
	i = deserialize_int(&y_pos, buf, i);

	i = deserialize_int(&x_vel, buf, i);
	i = deserialize_int(&y_vel, buf, i);

	i = deserialize_int(&radius, buf, i);

	return i;
}
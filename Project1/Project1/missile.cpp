#include <string>

#include "missile.h"

Missile* init_missile(int x_pos, int y_pos, int x_vel, int y_vel, int damage, int base_knockback, int knockback_scaling) {
	Missile* miss = new Missile;

	miss->x_pos = x_pos;
	miss->y_pos = y_pos;

	miss->x_vel = x_vel;
	miss->y_vel = y_vel;

	miss->damage = damage;
	miss->base_knockback = base_knockback;
	miss->knockback_scaling = knockback_scaling;

	miss->players_hit[0] = false;
	miss->players_hit[1] = false;
	miss->players_hit[2] = false;
	miss->players_hit[3] = false;

	return miss;
}

Missile** spawn_missiles(int gun_dir_x, int gun_dir_y, int x_pos, int y_pos, int velocity, int spread, int damage, int base_knockback, int knockback_scaling) {


	double spread_angle = 3.14159 * 0.2;

	Missile** new_missiles = (Missile**)malloc(sizeof(Missile*) * spread);

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
				new_missiles[i] = init_missile(x_pos, y_pos, x_vel, y_vel, damage, base_knockback, knockback_scaling);
			}
		}
	}

	return new_missiles;

}
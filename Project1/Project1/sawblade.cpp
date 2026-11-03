#include <string>

#include "serializer.h"
#include "sawblade.h"

Sawblade::Sawblade(int x, int y, int x_v, int y_v) {
	x_pos = x;
	y_pos = y;

	x_vel = x_v;
	y_vel = y_v;

	for (int i = 0; i < 4; i++) {
		hit_cooldowns[i] = 0;
	}
}

int Sawblade::serialize(char* buf, int i) {
	i = serialize_int(x_pos, buf, i);
	i = serialize_int(y_pos, buf, i);

	i = serialize_int(x_vel, buf, i);
	i = serialize_int(y_vel, buf, i);

	i = serialize_int((int)spin, buf, i);

	return i;
}

int Sawblade::deserialize(char* buf, int i) {
	i = deserialize_int(&x_pos, buf, i);
	i = deserialize_int(&y_pos, buf, i);

	i = deserialize_int(&x_vel, buf, i);
	i = deserialize_int(&y_vel, buf, i);

	int s;
	i = deserialize_int(&s, buf, i);
	spin = s;

	return i;
}

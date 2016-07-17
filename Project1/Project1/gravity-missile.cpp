#include "gravity-missile.h"

#include "serializer.h"

Gravity_Missile::Gravity_Missile() {}

Gravity_Missile::Gravity_Missile(int x, int y, int x_v, int y_v) {
	x_pos = x;
	y_pos = y;
	x_vel = x_v;
	y_vel = y_v;
	damage = 1;
}

int Gravity_Missile::serialize(char* buf, int i) {
	i = serialize_int(x_pos, buf, i);
	i = serialize_int(y_pos, buf, i);

	i = serialize_int(x_vel, buf, i);
	i = serialize_int(y_vel, buf, i);

	i = serialize_bool(exploded, buf, i);
	i = serialize_double(vortex_angle, buf, i);
	i = serialize_int(radius, buf, i);

	return i;
}

int Gravity_Missile::deserialize(char* buf, int i) {
	i = deserialize_int(&x_pos, buf, i);
	i = deserialize_int(&y_pos, buf, i);

	i = deserialize_int(&x_vel, buf, i);
	i = deserialize_int(&y_vel, buf, i);

	i = deserialize_bool(&exploded, buf, i);
	i = deserialize_double(&vortex_angle, buf, i);
	i = deserialize_int(&radius, buf, i);

	return i;
}
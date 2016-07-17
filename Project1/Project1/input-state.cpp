#include <SDL.h>

#include "serializer.h"
#include "input-state.h"

// todo: change buton serialization to use one byte per button
inline int Input_State::serialize_button(button b, char* buf, int i) {
	i = serialize_bool(b.changed, buf, i);
	i = serialize_bool(b.state, buf, i);

	return i;
}

inline int Input_State::deserialize_button(button b, char* buf, int i) {
	i = deserialize_bool(&b.changed, buf, i);
	i = deserialize_bool(&b.state, buf, i);

	return i;
}

int Input_State::serialize(char* buf, int i) {
	i = serialize_int(r_stick_x, buf, i);
	i = serialize_int(r_stick_y, buf, i);

	i = serialize_int(l_stick_x, buf, i);
	i = serialize_int(l_stick_y, buf, i);

	i = serialize_button(a, buf, i);
	i = serialize_button(b, buf, i);
	i = serialize_button(x, buf, i);
	i = serialize_button(y, buf, i);

	i = serialize_button(d_u, buf, i);
	i = serialize_button(d_d, buf, i);
	i = serialize_button(d_l, buf, i);
	i = serialize_button(d_r, buf, i);

	i = serialize_button(start, buf, i);
	i = serialize_button(select, buf, i);
	i = serialize_button(xbox, buf, i);

	i = serialize_button(rb, buf, i);
	i = serialize_button(rt, buf, i);

	i = serialize_button(lb, buf, i);
	i = serialize_button(lt, buf, i);

	i = serialize_button(r3, buf, i);
	i = serialize_button(l3, buf, i);

	return i;
}

int Input_State::deserialize(char* buf, int i) {
	i = deserialize_int(&r_stick_x, buf, i);
	i = deserialize_int(&r_stick_y, buf, i);

	i = deserialize_int(&l_stick_x, buf, i);
	i = deserialize_int(&l_stick_y, buf, i);

	i = deserialize_button(a, buf, i);
	i = deserialize_button(b, buf, i);
	i = deserialize_button(x, buf, i);
	i = deserialize_button(y, buf, i);

	i = deserialize_button(d_u, buf, i);
	i = deserialize_button(d_d, buf, i);
	i = deserialize_button(d_l, buf, i);
	i = deserialize_button(d_r, buf, i);

	i = deserialize_button(start, buf, i);
	i = deserialize_button(select, buf, i);
	i = deserialize_button(xbox, buf, i);

	i = deserialize_button(rb, buf, i);
	i = deserialize_button(rt, buf, i);

	i = deserialize_button(lb, buf, i);
	i = deserialize_button(lt, buf, i);

	i = deserialize_button(r3, buf, i);
	i = deserialize_button(l3, buf, i);

	return i;
}
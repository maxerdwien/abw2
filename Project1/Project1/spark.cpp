#include <string>

#include "serializer.h"
#include "spark.h"

Spark::Spark() {}

Spark::Spark(int x, int y) {
	x_1 = x + 10 * 10000 * ((double)rand() / RAND_MAX - 0.5);
	y_1 = y + 10 * 10000 * ((double)rand() / RAND_MAX - 0.5);

	x_2 = x_1 + 15 * 10000 * ((double)rand() / RAND_MAX - 0.5);
	y_2 = y_1 + 15 * 10000 * ((double)rand() / RAND_MAX - 0.5);

	vel = 10 * 10000 * (double)rand() / RAND_MAX;

	remaining_life = 5 + 10 * (double)rand() / RAND_MAX;
}

int Spark::serialize(char* buf, int i) {
	i = serialize_int(x_1, buf, i);
	i = serialize_int(y_1, buf, i);
	i = serialize_int(x_2, buf, i);
	i = serialize_int(y_2, buf, i);
	
	return i;
}

int Spark::deserialize(char* buf, int i) {
	i = deserialize_int(&x_1, buf, i);
	i = deserialize_int(&y_1, buf, i);
	i = deserialize_int(&x_2, buf, i);
	i = deserialize_int(&y_2, buf, i);

	return i;
}
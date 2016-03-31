#include <string>

#include "spark.h"

Spark::Spark(int x, int y) {
	x_1 = x;
	y_1 = y;

	x_2 = x + 15 * 10000 * ((double)rand() / RAND_MAX - 0.5);
	y_2 = y + 15 * 10000 * ((double)rand() / RAND_MAX - 0.5);

	vel = 10 * 10000 * (double)rand() / RAND_MAX;

	remaining_life = 5 + 10 * (double)rand() / RAND_MAX;
}
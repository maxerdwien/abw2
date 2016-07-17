#pragma once

class Spark {
public:
	int remaining_life;

	int x_1;
	int y_1;

	int x_2;
	int y_2;

	int vel;

	Spark(int x, int y);
	Spark();

	int serialize(char* buf, int i);
	int deserialize(char* buf, int i);
};
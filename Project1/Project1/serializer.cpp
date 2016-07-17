#include "serializer.h"

#include <cstring>
#include <string>

int serialize_int(int x, char* buffer, int index) {
	if (index + sizeof(int) >= 4096) {
		printf("cannot serialize; above limit of 4096 bytes\n");
		return -1;
	}
	std::memcpy(buffer + index, &x, sizeof(int));
	index += sizeof(int);
	
	return index;
}

int deserialize_int(int* x, char* buffer, int index) {
	std::memcpy(x, buffer + index, sizeof(int));
	index += sizeof(int);
	return index;
}

int serialize_bool(bool x, char* buffer, int index) {
	if (index + sizeof(bool) >= 4096) {
		printf("cannot serialize; above limit of 4096 bytes\n");
		return -1;
	}
	std::memcpy(buffer + index, &x, sizeof(bool));
	index += sizeof(bool);

	return index;
}

int deserialize_bool(bool* x, char* buffer, int index) {
	std::memcpy(x, buffer + index, sizeof(bool));
	index += sizeof(bool);
	return index;
}

int serialize_double(double x, char* buffer, int index) {
	if (index + sizeof(double) >= 4096) {
		printf("cannot serialize; above limit of 4096 bytes\n");
		return -1;
	}
	std::memcpy(buffer + index, &x, sizeof(double));
	index += sizeof(double);

	return index;
}

int deserialize_double(double* x, char* buffer, int index) {
	std::memcpy(x, buffer + index, sizeof(double));
	index += sizeof(double);
	return index;
}
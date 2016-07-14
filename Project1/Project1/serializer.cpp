#include "serializer.h"

#include <cstring>

int serialize_int(int x, char* buffer, int index) {
	std::memcpy(buffer + index, &x, sizeof(int));
	index += sizeof(int);
	return index;
}

int deserialize_int(int* x, char*buffer, int index) {
	std::memcpy(x, buffer + index, sizeof(int));
	index += sizeof(int);
	return index;
}
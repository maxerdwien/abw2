#include "serializer.h"

#include <cstring>
#include <string>

int serialize_int(int x, char* buffer, int index) {
	if (index + sizeof(int) >= 1024) {
		printf("cannot serialize; above limit of 1024 bytes");
		return -1;
	}
	std::memcpy(buffer + index, &x, sizeof(int));
	index += sizeof(int);
	
	return index;
}

int deserialize_int(int* x, char*buffer, int index) {
	std::memcpy(x, buffer + index, sizeof(int));
	index += sizeof(int);
	return index;
}
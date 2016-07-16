#pragma once

int serialize_int(int x, char* buffer, int index);

int deserialize_int(int* x, char*buffer, int index);

int serialize_bool(bool x, char* buffer, int index);

int deserialize_bool(bool* x, char* buffer, int index);
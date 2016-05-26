#pragma once

#include <SDL.h>

struct button {
	bool state = false;
	bool changed = false;
};

struct input_state {
	int r_stick_x = 0;
	int r_stick_y = 0;

	int l_stick_x = 0;
	int l_stick_y = 0;

	button a;
	button b;
	button x;
	button y;

	button d_u;
	button d_d;
	button d_l;
	button d_r;

	button start;
	button select;
	button xbox;

	button rb;
	button rt;

	button lb;
	button lt;

	button r3;
	button l3;

	bool occupied = false;
	SDL_GameController* controller = NULL;

	bool ai = false;
};
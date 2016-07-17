#pragma once

enum player_status {
	empty,
	human,
	ai,
	client,
};

struct button {
	bool state = false;
	bool changed = false;
};

class Input_State {
public:
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

	player_status status = empty;
	SDL_GameController* controller = NULL;

	int serialize_button(button b, char* buf, int i);
	int deserialize_button(button b, char* buf, int i); 
	
	int serialize(char* buf, int i);
	int deserialize(char* buf, int i);
};
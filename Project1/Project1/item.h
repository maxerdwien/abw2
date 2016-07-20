#pragma once

enum item_type : int {
	shield,
	laser_sights,
	speed_up,
	bounce,
	small_pwrup,
	bullet_bounce,

	NUM_ITEM_TYPES
};

class Item {
private:
	Renderer* r;
	SDL_Texture* tex;
	std::string text;

public:
	int x_pos;
	int y_pos;

	const int radius = 20 * 10000;

	bool picked_up = false;

	int alpha = 255;

	item_type type;

	Item(Renderer* rend);
	Item(int x, int y, item_type t, Renderer* rend);

	void get_resources();

	void render();

	int serialize(char* buf, int i);
	int deserialize(char* buf, int i);
};
#pragma once

enum item_type {
	shield,
	laser_sights,
	speed_up,
	bounce,
	small,
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

	int radius = 20 * 10000;

	bool picked_up = false;

	int alpha = 255;

	item_type type;

	Item(int x, int y, item_type t, Renderer* rend);

	void render();
};

#include "renderer.h"

#include "item.h"
Item::Item(int x, int y, item_type t, Renderer* rend) {
	x_pos = x;
	y_pos = y;

	type = t;

	r = rend;

	tex = r->LoadTexture("..\\Project1\\assets\\shield.png");
}

void Item::render() {
	r->render_texture_abs_size(tex, x_pos, y_pos, 0, radius);
}

#include "renderer.h"

#include "item.h"
Item::Item(int x, int y, item_type t, Renderer* rend) {
	x_pos = x;
	y_pos = y;

	type = t;

	r = rend;

	switch (type) {
	case shield:
		tex = r->LoadTexture("..\\Project1\\assets\\powerUpTeal.png");
		break;
	case laser_sights:
		tex = r->LoadTexture("..\\Project1\\assets\\powerUpGrey.png");
		break;
	case speed_up:
		tex = r->LoadTexture("..\\Project1\\assets\\powerUpPurple.png");
		break;
	case bounce:
		tex = r->LoadTexture("..\\Project1\\assets\\powerUpOrange.png");
		break;
	case small:
		tex = r->LoadTexture("..\\Project1\\assets\\powerUpYellow.png");
		break;
	case bullet_bounce:
		tex = r->LoadTexture("..\\Project1\\assets\\powerUpRed.png");
		break;
	}
}

void Item::render() {
	//r->render_texture_abs_size(tex, x_pos, y_pos, 0, radius);
	r->render_texture(tex, x_pos, y_pos, 0, 2);
}
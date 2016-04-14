
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
		text = "shield blocks damage!";
		break;
	case laser_sights:
		tex = r->LoadTexture("..\\Project1\\assets\\powerUpGrey.png");
		text = "laser sights!";
		break;
	case speed_up:
		tex = r->LoadTexture("..\\Project1\\assets\\powerUpPurple.png");
		text = "speed up!";
		break;
	case bounce:
		tex = r->LoadTexture("..\\Project1\\assets\\powerUpOrange.png");
		text = "bounce off edges!";
		break;
	case small:
		tex = r->LoadTexture("..\\Project1\\assets\\powerUpYellow.png");
		text = "hard to hit!";
		break;
	case bullet_bounce:
		tex = r->LoadTexture("..\\Project1\\assets\\powerUpRed.png");
		text = "bouncy bullets!";
		break;
	}
}

void Item::render() {
	if (picked_up) {
		r->render_text(x_pos, y_pos, text, true, true, false, small_f, alpha);
	} else {
		r->render_texture(tex, x_pos, y_pos, 0, 2);
	}
}
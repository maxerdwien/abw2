#include <SDL_image.h>
#include <string>

#include "renderer.h"
#include "stage_marker.h"

Stage_Marker::Stage_Marker(Renderer* rend, int x, int y, std::string n, std::string desc) {
	r = rend;
	
	x_pos = x;
	y_pos = y;

	selected = false;

	selection_radius = 10000 * 60;
	attraction_radius = 10000 * 90;

	name = n;
	description = desc;

	unselected_tex = r->LoadTexture("..\\Project1\\assets\\hitbox.png");
	selected_tex = r->LoadTexture("..\\Project1\\assets\\sun.png");
}

void Stage_Marker::render() {
	SDL_Texture* tex;
	if (selected) {
		tex = selected_tex;
	} else {
		tex = unselected_tex;
	}
	r->render_texture(tex, x_pos, y_pos, 0, 0.3);

	if (selected) {
		int text_offset = 10000 * 40;
		r->render_text(x_pos, y_pos + text_offset, name, true, false, true, font_size::medium_f, 255, 255);
		r->render_text(x_pos, y_pos + text_offset + r->get_text_height(font_size::medium_f), description, true, false, true, font_size::small_f, 255, 255);
	}
}
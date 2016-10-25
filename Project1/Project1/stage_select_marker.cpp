#include <SDL_image.h>

#include "renderer.h"
#include "stage_select_marker.h"

Stage_Select_Marker::Stage_Select_Marker(Renderer* rend) {
	r = rend;
	x_pos = 0;
	y_pos = 0;
}

void Stage_Select_Marker::render() {
	int line_length = 1000000;

	r->SetRenderDrawColor(255, 255, 255, SDL_ALPHA_OPAQUE);
	
	r->render_line_w_end(x_pos - line_length / 2, y_pos - line_length / 2, x_pos + line_length / 2, y_pos + line_length / 2);
	r->render_line_w_end(x_pos + line_length / 2, y_pos - line_length / 2, x_pos - line_length / 2, y_pos + line_length / 2);
}
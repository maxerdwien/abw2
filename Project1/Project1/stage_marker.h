#pragma once

class Stage_Marker {
private:
	Renderer* r;
	SDL_Texture* unselected_tex;
	SDL_Texture* selected_tex;

	std::string name;

public:
	int x_pos;
	int y_pos;

	bool selected;

	int selection_radius;
	int attraction_radius;

	Stage_Marker(Renderer* rend, int x, int y, std::string n);

	void render();
};
#pragma once

class Stage_Select_Marker {
private:
	class Renderer* r;
public:
	int x_pos;
	int y_pos;

	Stage_Select_Marker(Renderer* rend);

	void render();
};
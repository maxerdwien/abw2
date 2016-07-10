#pragma once

class weapon {
public:
	virtual void fire();
//	virtual void update(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]);
	virtual void render();
};
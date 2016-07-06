#pragma once

class flamethrower : weapon {

public:
	bool flame_active;
	const int flame_switch_delay = 5;
	int flame_switch_cooldown = 0;
	int current_flame = 0;

	void fire();
	void update(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]);
	void render();
};


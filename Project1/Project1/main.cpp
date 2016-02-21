#include <iostream>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>

#include "spaceship.h"
#include "bullet.h"
#include "body.h"

void render_texture(SDL_Texture*, SDL_Renderer*, int x, int y, double angle);
double calculate_angle(int x_vel, int y_vel);
bullet* spawn_bullets(spaceship* ship, int velocity, int spread, int damage);

int main(int, char**) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0) {
		std::cout << "SDL_Init error: " << SDL_GetError() << std::endl;
		return 1;
	}
	enum Resolution {
		_480p,
		_720p,
		_1080p
	};
	const Resolution res = _720p;
	int WINDOW_WIDTH;
	int WINDOW_HEIGHT;
	switch (res) {
	case _480p:
		WINDOW_WIDTH = 640;
		WINDOW_HEIGHT = 480;
		break;
	case _720p:
		WINDOW_WIDTH = 1280;
		WINDOW_HEIGHT = 720;
		break;
	case _1080p:
		WINDOW_WIDTH = 1920;
		WINDOW_HEIGHT = 1080;
		break;
	default:
		WINDOW_WIDTH = 1280;
		WINDOW_HEIGHT = 720;
	}
	
	
	const int DEAD_ZONE = 8000;

	const bool DO_HAPTIC = false;

	//const Uint32 FRAME_TIME = 1000 / 60;
	SDL_Window* window = SDL_CreateWindow("hl2.exe", 1000, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (!window) {
		std::cout << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		SDL_DestroyWindow(window);
		std::cout << "SDL_CreateRenderer error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	// register controllers
	SDL_GameController* controllers[4];
	SDL_Haptic* haptics[4];
	int num_joysticks = SDL_NumJoysticks();
	//std::cout << num_joysticks << std::endl;
	for (int i = 0; i < num_joysticks; i++) {
		controllers[i] = NULL;
		{
			if (SDL_IsGameController(i)) {
				controllers[i] = SDL_GameControllerOpen(0);
			}
			if (!controllers[i]) {
				std::cout << "could not connect controller" << std::endl;
			}
		}
		haptics[i] = SDL_HapticOpen(i);
		{
			if (!haptics[i]) {
				std::cout << "could not connect haptic device" << std::endl;
			}
			if (SDL_HapticRumbleInit(haptics[i]) != 0) {
				std::cout << "could not init haptic device" << std::endl;
			}
		}
	}

	SDL_Texture* bg = IMG_LoadTexture(renderer, "..\\Project1\\assets\\background.png");
	if (!bg) {
		std::cout << "error loading background: " << SDL_GetError() << std::endl;
	}
	SDL_Texture* spaceship = IMG_LoadTexture(renderer, "..\\Project1\\assets\\spaceship.png");
	SDL_Texture* spaceship_low = IMG_LoadTexture(renderer, "..\\Project1\\assets\\spaceship_low.png");
	SDL_Texture* spaceship_high = IMG_LoadTexture(renderer, "..\\Project1\\assets\\spaceship_high.png");
	//SDL_Texture* spaceship = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bear-idle.png");
	//SDL_Texture* spaceship_low = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bear-idle.png");
	//SDL_Texture* spaceship_high = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bear-idle.png");
	
	SDL_Texture* sun_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\sun.png");
	SDL_Texture* bullet_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bullet.png");
	SDL_Texture* cannon = IMG_LoadTexture(renderer, "..\\Project1\\assets\\cannon.png");

	SDL_Event e;
	bool quit = false;
	bool pause = false;

	const int num_players = 4;
	struct spaceship* ships[num_players];
	ships[0] = init_spaceship(100*10000, 100*10000);
	ships[1] = init_spaceship(500*10000, 100*10000);
	ships[2] = init_spaceship(100*10000, 500*10000);
	ships[3] = init_spaceship(500*10000, 500*10000);

	struct body* sun = new body;
	sun->x_pos = WINDOW_WIDTH / 2 * 10000;
	sun->y_pos = WINDOW_HEIGHT / 2 * 10000;

	int sun_mass = 1000;

	double G = 5000000000000;

	Uint32 last_frame_start_time = SDL_GetTicks();
	Uint32 frame_start_time = SDL_GetTicks();

	while (!quit) {
		last_frame_start_time = frame_start_time;
		frame_start_time = SDL_GetTicks();
		int controller_index;
		struct spaceship* ship;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				controller_index = e.cbutton.which;
				ship = ships[controller_index];
				if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
					ship->x_vel = 0;
					ship->y_vel = 0;
				}
				else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
					ship->x_vel = 0;
					ship->y_vel = 0;
					ship->x_pos = 0;
					ship->y_pos = 0;
				}
				else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_X) {

				}
				else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_Y) {

				}
				else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
					ship->fire_normal = true;
				}
				else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
					ship->fire_burst = true;
				}
				else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
					pause = !pause;
				}
				else {
					quit = true;
				}
				break;
			case SDL_CONTROLLERBUTTONUP:
				controller_index = e.cbutton.which;
				ship = ships[controller_index];
				if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
					ship->fire_normal = false;
				} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
					ship->fire_burst = false;
				}
				break;
			case SDL_CONTROLLERAXISMOTION:
				controller_index = e.caxis.which;
				ship = ships[controller_index];
				if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
					if (abs(e.caxis.value) > DEAD_ZONE) {
						ship->move_dir_x = (int)e.caxis.value;
					}
					else {
						ship->move_dir_x = 0;
					}
				}
				if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
					if (abs(e.caxis.value) > DEAD_ZONE) {
						ship->move_dir_y = (int)e.caxis.value;
					}
					else {
						ship->move_dir_y = 0;
					}
					
				}

				if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
					double new_gun_mag = sqrt(pow(e.caxis.value, 2) + pow(ship->gun_dir_y, 2));
					if (new_gun_mag >= ship->MIN_GUN_DIR) {
						ship->gun_dir_x = (int)(e.caxis.value);
					}
					else {
						ship->gun_dir_x = 0;
					}
				}
				if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
					double new_gun_mag = sqrt(pow(e.caxis.value, 2) + pow(ship->gun_dir_x, 2));
					if (new_gun_mag >= ship->MIN_GUN_DIR) {
						ship->gun_dir_y = (int)(e.caxis.value);
					}
					else {
						ship->gun_dir_y = 0;
					}
				}
			break;
			}

		}
		if (pause) {
			continue;
		}

		for (int i = 0; i < num_players; i++) {
			struct spaceship* ship = ships[i];
			// handle bullet spawns
			if (ship->cannon_cooldown > 0) {
				ship->cannon_cooldown -= (frame_start_time - last_frame_start_time);
			}
			if (ship->fire_normal && ship->cannon_cooldown <= 0) {

				int MUZZLE_VEL = 50000;
				int spread = 1;
				bullet* new_bullets = spawn_bullets(ship, MUZZLE_VEL, spread, 2);
				for (int i = 0; i < spread; i++) {
					ship->bullets[ship->num_bullets] = &new_bullets[i];
					ship->num_bullets++;

				}
				ship->cannon_cooldown += ship->cannon_delay;

			}

			// handle burst fire shots
			if (ship->burst_cooldown_1 > 0) {
				ship->burst_cooldown_1 -= (frame_start_time - last_frame_start_time);
			}
			if (ship->burst_shot_current != 0 && ship->burst_cooldown_2 > 0) {
				ship->burst_cooldown_2 -= (frame_start_time - last_frame_start_time);
			}
			if (ship->fire_burst && ship->burst_cooldown_1 <= 0) {
				int MUZZLE_VEL = 20000;
				int spread = 1;
				bullet* new_bullets = spawn_bullets(ship, MUZZLE_VEL, spread, 1);
				for (int i = 0; i < spread; i++) {
					ship->bullets[ship->num_bullets] = &new_bullets[i];
					ship->num_bullets++;
				}
				ship->burst_cooldown_1 += ship->burst_delay_1;
				ship->burst_shot_current++;
			}
			if (ship->burst_cooldown_2 <= 0) {
				int MUZZLE_VEL = 20000;
				int spread = 1;
				bullet* new_bullets = spawn_bullets(ship, MUZZLE_VEL, spread, 1);
				for (int i = 0; i < spread; i++) {
					ship->bullets[ship->num_bullets] = &new_bullets[i];
					ship->num_bullets++;
				}
				ship->burst_cooldown_2 += ship->burst_delay_2;
				ship->burst_shot_current++;
				if (ship->burst_shot_current == ship->burst_shot_number) {
					ship->burst_shot_current = 0;
				}
			}

			// update ship
			{

				// update acceleration
				double original_vel = pow(ship->x_vel, 2) + pow(ship->y_vel, 2);
				ship->x_vel = 2 * ship->move_dir_x;
				ship->y_vel = 2 * ship->move_dir_y;
				double new_vel = pow(ship->x_vel, 2) + pow(ship->y_vel, 2);
				std::cout << new_vel << std::endl;

				/*
				ship->x_vel_accumulator = 0;
				ship->y_vel_accumulator = 0;
				//std::cout << ship->move_dir_x << std::endl;
				double accel_mag = sqrt(pow(ship->move_dir_x, 2) + pow(ship->move_dir_y, 2));
				if (accel_mag > SPACESHIP_MAX_ACCEL) {
					accel_mag = SPACESHIP_MAX_ACCEL;
				}
				if (ship->move_dir_x != 0 || ship->move_dir_y != 0) {
					ship->x_accel = accel_mag*ship->move_dir_x / sqrt(pow(ship->move_dir_x, 2) + pow(ship->move_dir_y, 2));
					ship->y_accel = accel_mag*ship->move_dir_y / sqrt(pow(ship->move_dir_x, 2) + pow(ship->move_dir_y, 2));
				}
				else {
					ship->x_accel = 0;
					ship->y_accel = 0;
					//ship->control_state = stabilize;
				}
					*/

				//ship->x_vel += ship->x_accel;
				//ship->y_vel += ship->y_accel;

				//std::cout << sqrt(pow(ship->move_dir_x, 2) + pow(ship->move_dir_y, 2)) << "\t" << ship->move_dir_x << std::endl;
				ship->x_pos += ship->x_vel;
				ship->y_pos += ship->y_vel;

				// do haptic feedback
				/*
				double total_accel = sqrt(pow(ship->x_accel + x_grav_accel, 2) + pow(ship->y_accel + y_grav_accel, 2));
				//std::cout << total_accel << std::endl;
				float amount = (total_accel - 3700) / 1200;
				//std::cout << amount << std::endl;
				if (amount > 1) {
					amount = 1;
				}
				if (amount > 0 && DO_HAPTIC == true) {
					if (SDL_HapticRumblePlay(haptic, amount, 100) != 0) {
						std::cout << "error while playing haptic effect" << std::endl;
					}
				}
				*/
			}

			// update bullets
			for (int j = 0; j < ship->num_bullets; j++) {
				struct bullet* bullet = ship->bullets[j];
				double dist = sqrt(pow(bullet->x_pos - sun->x_pos, 2) + pow(bullet->y_pos - sun->y_pos, 2));

				double power = G * sun_mass / pow(dist, 2);
				double ratio = (double)power / (double)dist;
				int x_grav_accel = (int)((sun->x_pos - bullet->x_pos) * ratio);
				//std::cout << dist << "\t" << (x_offset - sun_x) * ratio << std::endl;
				int y_grav_accel = (int)((sun->y_pos - bullet->y_pos) * ratio);

				//bullets[i]->x_vel += bullets[i]->x_accel + x_grav_accel;
				//bullets[i]->y_vel += bullets[i]->y_accel + y_grav_accel;
				bullet->x_vel += bullet->x_accel;
				bullet->y_vel += bullet->y_accel;
				bullet->x_pos += bullet->x_vel;
				bullet->y_pos += bullet->y_vel;
				//std::cout << ship->x_pos << std::endl;

				// check for collisions with enemies
				for (int k = 0; k < num_players; k++) {
					if (i == k) continue;
					double dist = sqrt(pow(bullet->x_pos - ships[k]->x_pos, 2) + pow(bullet->y_pos - ships[k]->y_pos, 2));
					//std::cout << dist << std::endl;
					if (dist <= 20 * 10000) {
						// todo: do haptic on hit

						

						ship->num_bullets--;
						free(ship->bullets[j]);
						ship->bullets[j] = ship->bullets[ship->num_bullets];
						i--;
						break;
					}
				}
			}
		}

		

		SDL_RenderClear(renderer);
		render_texture(bg, renderer, WINDOW_WIDTH/2, WINDOW_HEIGHT/2, 0);
		render_texture(sun_tex, renderer, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0);

		// render ships
		for (int i = 0; i < num_players; i++) {
			struct spaceship* ship = ships[i];
			double total_engine = sqrt(pow(ship->x_accel, 2) + pow(ship->y_accel, 2));
			double angle = calculate_angle(ship->x_accel, ship->y_accel);
			if (total_engine <= 500) {
				render_texture(spaceship, renderer, ship->x_pos / 10000, ship->y_pos / 10000, angle);
			}
			else if (total_engine <= 2000) {
				render_texture(spaceship_low, renderer, ship->x_pos / 10000, ship->y_pos / 10000, angle);
			}
			else {
				render_texture(spaceship_high, renderer, ship->x_pos / 10000, ship->y_pos / 10000, angle);
			}

			{
				SDL_Rect rect;

				SDL_QueryTexture(cannon, NULL, NULL, &rect.w, &rect.h);
				rect.x = ship->x_pos / 10000 - rect.w;
				rect.y = ship->y_pos / 10000 - rect.h;
				//std::cout << rect.x << std::endl;
				SDL_Point* point = new SDL_Point;
				point->x = 2;
				point->y = 11;
				SDL_RenderCopyEx(renderer, cannon, NULL, &rect, calculate_angle(ship->gun_dir_x, ship->gun_dir_y), point, SDL_FLIP_NONE);
				free(point);
			}

			// render bullets
			for (int i = 0; i < ship->num_bullets; i++) {
				double angle = calculate_angle(ship->bullets[i]->x_vel, ship->bullets[i]->y_vel);
				//std::cout << bullets[i]->x_vel << "\t" << bullets[i]->y_vel << "\t" << angle << std::endl;
				render_texture(bullet_tex, renderer, ship->bullets[i]->x_pos / 10000, ship->bullets[i]->y_pos / 10000, angle);
			}
		}
		
		SDL_RenderPresent(renderer);
	}

	for (int i = 0; i < num_joysticks; i++) {
		// todo: close the controllers themselves?
		SDL_HapticClose(haptics[i]);
	}
	SDL_DestroyTexture(bg);
	// todo: destroy other textures
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}

bullet* spawn_bullets(spaceship* ship, int velocity, int spread, int damage) {
	

	double spread_angle = 3.14159 * 0.2;

	bullet* new_bullets = (bullet*)malloc(sizeof(bullet) * spread); // todo: free me

	if (spread % 2 == 0) {

	}
	else {
		double start_angle = spread_angle * (spread - 1) / 2;
		double straight_x_vel = ship->gun_dir_x / sqrt(pow(ship->gun_dir_x, 2) + pow(ship->gun_dir_y, 2));
		double straight_y_vel = ship->gun_dir_y / sqrt(pow(ship->gun_dir_x, 2) + pow(ship->gun_dir_y, 2));

		for (int i = 0; i < spread; i++) {
			double y_over_x = tan(atan2(straight_y_vel, straight_x_vel) - 0);
			//std::cout << y_over_x << "\t" << straight_y_vel / straight_x_vel << std::endl;

			//double x_vel = velocity*sqrt(1 / (1 + pow(y_over_x, 2)));
			//std::cout << 1.0 - pow(x_vel, 2) << std::endl;
			//double y_vel = velocity*sqrt(1.0 - pow(x_vel, 2));
			//double y_vel = velocity*sqrt(1 / ((1 / y_over_x) + 1));

			int x_vel = velocity*straight_x_vel;
			int y_vel = velocity*straight_y_vel;

			new_bullets[i] = init_bullet(ship->x_pos, ship->y_pos, x_vel, y_vel, damage)[0];
		}
	}

	return new_bullets;
	
}

double calculate_angle(int x_vel, int y_vel) {
	double angle;
	double conversion = 180 / M_PI;
	angle = atan2(y_vel, x_vel) * conversion + 90;
	return angle;
}

void render_texture(SDL_Texture* texture, SDL_Renderer* renderer, int x, int y, double angle) {
	SDL_Rect rect;
	
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	rect.x = x-rect.w/2;
	rect.y = y-rect.h/2;
	SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
}
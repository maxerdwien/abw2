#include <iostream>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>

#include "spaceship.h"
#include "bullet.h"
#include "baddie.h"
#include "body.h"

void render_texture(SDL_Texture*, SDL_Renderer*, int x, int y, double angle);
double calculate_angle(int x_vel, int y_vel);
bullet* spawn_bullets(spaceship* ship, int velocity, int spread);

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
	bool DO_SHIP_GRAVITY = true;

	//const Uint32 FRAME_TIME = 1000 / 60;
	SDL_Window* window = SDL_CreateWindow("window_name", 1000, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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

	// register controller
	SDL_GameController* controller = NULL;
	{
		//std::cout << SDL_NumJoysticks() << std::endl;
		if (SDL_IsGameController(0)) {
			controller = SDL_GameControllerOpen(0);
		}
		if (!controller) {
			std::cout << "could not connect controller" << std::endl;
		}
	}
	SDL_Haptic* haptic = SDL_HapticOpen(0);
	{
		if (!haptic) {
			std::cout << "could not connect haptic device" << std::endl;
		}
		if (SDL_HapticRumbleInit(haptic) != 0) {
			std::cout << "could not init haptic device" << std::endl;
		}
	}

	SDL_Texture* bg = IMG_LoadTexture(renderer, "..\\Project1\\assets\\background.png");
	if (!bg) {
		std::cout << "error loading background: " << SDL_GetError() << std::endl;
	}
	//SDL_Texture* spaceship = IMG_LoadTexture(renderer, "..\\Project1\\assets\\spaceship.png");
	//SDL_Texture* spaceship_low = IMG_LoadTexture(renderer, "..\\Project1\\assets\\spaceship_low.png");
	//SDL_Texture* spaceship_high = IMG_LoadTexture(renderer, "..\\Project1\\assets\\spaceship_high.png");
	SDL_Texture* spaceship = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bear-idle.png");
	SDL_Texture* spaceship_low = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bear-idle.png");
	SDL_Texture* spaceship_high = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bear-idle.png");
	
	SDL_Texture* sun_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\sun.png");
	SDL_Texture* bullet_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bullet.png");
	SDL_Texture* cannon = IMG_LoadTexture(renderer, "..\\Project1\\assets\\cannon.png");
	SDL_Texture* baddie = IMG_LoadTexture(renderer, "..\\Project1\\assets\\baddie.png");

	SDL_Event e;
	bool quit = false;
	bool pause = false;

	struct spaceship* ship = init_spaceship();

	struct bullet* bullets[1000];
	int num_bullets = 0;

	struct baddie* baddies[1000];
	int num_baddies = 0;
	int spawn_cooldown = 0;
	int spawn_delay = 1000;

	struct body* sun = new body;
	sun->x_pos = WINDOW_WIDTH / 2 * 10000;
	sun->y_pos = WINDOW_HEIGHT / 2 * 10000;

	int sun_mass = 1000;

	double G = 5000000000000;

	//bool fire_gun = false;
	bool fire_gun = true;

	Uint32 last_frame_start_time = SDL_GetTicks();
	Uint32 frame_start_time = SDL_GetTicks();

	while (!quit) {
		last_frame_start_time = frame_start_time;
		frame_start_time = SDL_GetTicks();
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				//std::cout << (int)e.cbutton.button << std::endl;
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
					if (ship->control_state == stabilize) {
						ship->control_state = manual;
					}
					else {
						ship->control_state = stabilize;
					}
				}
				else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_Y) {
					if (ship->control_state == orbit) {
						ship->control_state = manual;
					}
					else {
						ship->control_state = orbit;
					}
				}
				else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
					fire_gun = true;
				}
				else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
					DO_SHIP_GRAVITY = !DO_SHIP_GRAVITY;
				}
				else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
					pause = !pause;
				}
				else {
					quit = true;
				}
				break;
			case SDL_CONTROLLERBUTTONUP:
				if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
					//fire_gun = false;
				}
				break;
			case SDL_CONTROLLERAXISMOTION:
				//std::cout << e.caxis.which << "\t" << e.caxis.axis << "\t" << e.caxis.value << std::endl;
				if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
					if (abs(e.caxis.value) > DEAD_ZONE) {
						ship->control_state = manual;
						ship->move_dir_x = (int)e.caxis.value;
					}
					else {
						ship->move_dir_x = 0;
					}
				}
				if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
					if (abs(e.caxis.value) > DEAD_ZONE) {
						ship->control_state = manual;
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
				//std::cout << ship->gun_dir_x << "\t" << ship->gun_dir_y << "\t" << sqrt(pow(ship->gun_dir_y, 2) + pow(ship->gun_dir_x, 2)) << std::endl;
				/*
				if (ship->gun_dir_x >= 0 && ship->gun_dir_x < 999) {
					ship->gun_dir_x = 1000;
				}
				if (ship->gun_dir_x < 0 && ship->gun_dir_x > -999) {
					ship->gun_dir_x = -1000;
				}
				if (ship->gun_dir_y >= 0 && ship->gun_dir_y < 999) {
					ship->gun_dir_y = 1000;
				}
				if (ship->gun_dir_y < 0 && ship->gun_dir_y > -999) {
					ship->gun_dir_y = -1000;
				}
				*/
			break;
			//default:
			//std::cout << e.type << std::endl;
			}

		}
		if (pause) {
			continue;
		}
		// handle baddie spawns
		if (spawn_cooldown > 0) {
			spawn_cooldown -= (frame_start_time - last_frame_start_time);
		}
		if (spawn_cooldown <= 0) {
			//std::cout << "spawning baddie" << std::endl;
			// get random spawn point on border of screen.
			int pos = rand() % ((WINDOW_WIDTH + WINDOW_HEIGHT) * 2);
			int x_pos;
			int y_pos;
			if (pos <= WINDOW_WIDTH) {
				y_pos = 0;
				x_pos = pos;
			}
			else if (pos <= WINDOW_WIDTH + WINDOW_HEIGHT) {
				x_pos = WINDOW_WIDTH;
				y_pos = pos - WINDOW_WIDTH;
			}
			else if (pos <= WINDOW_WIDTH * 2 + WINDOW_HEIGHT) {
				x_pos = pos - WINDOW_WIDTH - WINDOW_HEIGHT;
				y_pos = WINDOW_HEIGHT;
			}
			else {
				x_pos = 0;
				y_pos = pos - WINDOW_WIDTH * 2 - WINDOW_HEIGHT;
			}
			x_pos *= 10000;
			y_pos *= 10000;

			float x_sun_dist = sun->x_pos - x_pos;
			float y_sun_dist = sun->y_pos - y_pos;

			float x_vel = BADDIE_MAX_VEL*x_sun_dist / sqrt(pow(y_sun_dist, 2) + pow(x_sun_dist, 2));
			float y_vel = BADDIE_MAX_VEL*y_sun_dist / sqrt(pow(y_sun_dist, 2) + pow(x_sun_dist, 2));

			baddies[num_baddies] = init_baddie(x_pos, y_pos, (int)x_vel, (int)y_vel);
			num_baddies++;
			spawn_cooldown += spawn_delay;
		}

		// handle bullet spawns
		if (ship->cannon_cooldown > 0) {
			ship->cannon_cooldown -= (frame_start_time - last_frame_start_time);
		}
		if (fire_gun && ship->cannon_cooldown <= 0) {
			int extra_x_vel = 0;
			int extra_y_vel = 0;

			int MUZZLE_VEL = 50000;
			int spread = 1;
			bullet* new_bullets = spawn_bullets(ship, MUZZLE_VEL, spread);
			for (int i = 0; i < spread; i++) {
				bullets[num_bullets] = &new_bullets[i];
				num_bullets++;
				
			}
			ship->cannon_cooldown += ship->cannon_delay;
			
		}

		// update ship
		{
			// calculate gravity
			if (DO_SHIP_GRAVITY) {
				double dist = sqrt(pow(ship->x_pos - sun->x_pos, 2) + pow(ship->y_pos - sun->y_pos, 2));
				double power = G * sun_mass / pow(dist, 2);
				double ratio = (double)power / (double)dist;
				int x_grav_accel = (int)((sun->x_pos - ship->x_pos) * ratio);
				int y_grav_accel = (int)((sun->y_pos - ship->y_pos) * ratio);

				ship->x_vel += x_grav_accel;
				ship->y_vel += y_grav_accel;
			}

			// update acceleration
			if (ship->control_state == manual) {
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
			} else if (ship->control_state == stabilize) {
				ship->x_vel_accumulator += (ship->x_vel * (frame_start_time - last_frame_start_time));
				ship->y_vel_accumulator += (ship->y_vel * (frame_start_time - last_frame_start_time));
				//ship->x_accel = -ship->x_vel;
				//ship->y_accel = -ship->y_vel;
				float kp = 0.001;
				float ki = 0.00005;
				ship->x_accel = -kp*ship->x_vel - ki*ship->x_vel_accumulator;
				ship->y_accel = -kp*ship->y_vel - ki*ship->y_vel_accumulator;
				if (sqrt(pow(ship->x_accel, 2) + pow(ship->y_accel, 2)) > SPACESHIP_MAX_ACCEL) {
					std::cout << "max accel exceeded " << rand() << std::endl;
					ship->x_accel = SPACESHIP_MAX_ACCEL*ship->x_accel / sqrt(pow(ship->x_accel, 2) + pow(ship->y_accel, 2));
					ship->y_accel = SPACESHIP_MAX_ACCEL*ship->y_accel / sqrt(pow(ship->x_accel, 2) + pow(ship->y_accel, 2));
				}
			}
			else if (ship->control_state == orbit) {
				double radial_angle = atan2(ship->y_pos - sun->y_pos, ship->x_pos - sun->x_pos);
				double vel_angle = atan2(ship->y_vel, ship->x_vel);
				std::cout << radial_angle - vel_angle << std::endl;
				double desired_angle = radial_angle;
				bool invert = false;
				if (radial_angle - vel_angle + M_PI/2 < 0) {
					// accelerate away from sun
					//desired_angle = radial_angle - M_PI / 2;
					invert = true;
					
					printf("away");
				}
				else {
					printf("towards");
				}

				double radial_vel = 0;
				ship->x_accel = SPACESHIP_MAX_ACCEL / (tan(desired_angle) + 1);
				ship->y_accel = SPACESHIP_MAX_ACCEL / (1/tan(desired_angle) + 1);
				if (invert) {
					ship->x_accel *= -1;
					ship->y_accel *= -1;
				}
			}
			
			ship->x_vel += ship->x_accel;
			ship->y_vel += ship->y_accel;

			//std::cout << sqrt(pow(ship->move_dir_x, 2) + pow(ship->move_dir_y, 2)) << "\t" << ship->move_dir_x << std::endl;
			ship->x_pos += ship->x_vel;
			ship->y_pos += ship->y_vel;
			//std::cout << ship->x_pos << std::endl;

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

		// update baddies
		for (int i = 0; i < num_baddies; i++) {
			baddies[i]->x_pos += baddies[i]->x_vel;
			baddies[i]->y_pos += baddies[i]->y_vel;
		}

		// update bullets
		for (int i = 0; i < num_bullets; i++) {
			double dist = sqrt(pow(bullets[i]->x_pos - sun->x_pos, 2) + pow(bullets[i]->y_pos - sun->y_pos, 2));

			double power = G * sun_mass / pow(dist, 2);
			double ratio = (double)power / (double)dist;
			int x_grav_accel = (int)((sun->x_pos - bullets[i]->x_pos) * ratio);
			//std::cout << dist << "\t" << (x_offset - sun_x) * ratio << std::endl;
			int y_grav_accel = (int)((sun->y_pos - bullets[i]->y_pos) * ratio);

			bullets[i]->x_vel += bullets[i]->x_accel + x_grav_accel;
			bullets[i]->y_vel += bullets[i]->y_accel + y_grav_accel;
			bullets[i]->x_pos += bullets[i]->x_vel;
			bullets[i]->y_pos += bullets[i]->y_vel;
			//std::cout << ship->x_pos << std::endl;

			// check for collisions with baddies
			for (int j = 0; j < num_baddies; j++) {
				double dist = sqrt(pow(bullets[i]->x_pos-baddies[j]->x_pos,2) + pow(bullets[i]->y_pos - baddies[j]->y_pos, 2));
				//std::cout << dist << std::endl;
				if (dist <= 20 * 10000) {
					std::cout << "baddie killed" << std::endl;
					num_baddies--;
					free(baddies[j]);
					baddies[j] = baddies[num_baddies];

					num_bullets--;
					free(bullets[i]);
					bullets[i] = bullets[num_bullets];
					i--;
					break;
				}
			}
		}

		SDL_RenderClear(renderer);
		render_texture(bg, renderer, WINDOW_WIDTH/2, WINDOW_HEIGHT/2, 0);
		render_texture(sun_tex, renderer, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0);
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
			rect.x = ship->x_pos/10000 - rect.w;
			rect.y = ship->y_pos/10000 - rect.h;
			//std::cout << rect.x << std::endl;
			SDL_Point* point = new SDL_Point;
			point->x = 2;
			point->y = 11;
			SDL_RenderCopyEx(renderer, cannon, NULL, &rect, calculate_angle(ship->gun_dir_x, ship->gun_dir_y), point, SDL_FLIP_NONE);
			free(point);
		}

		for (int i = 0; i < num_baddies; i++) {
			double angle = calculate_angle(baddies[i]->x_vel, baddies[i]->y_vel);
			render_texture(baddie, renderer, baddies[i]->x_pos / 10000, baddies[i]->y_pos / 10000, angle);
		}
		
		for (int i = 0; i < num_bullets; i++) {
			double angle = calculate_angle(bullets[i]->x_vel, bullets[i]->y_vel);
			//std::cout << bullets[i]->x_vel << "\t" << bullets[i]->y_vel << "\t" << angle << std::endl;
			render_texture(bullet_tex, renderer, bullets[i]->x_pos / 10000, bullets[i]->y_pos / 10000, angle);
		}
		SDL_RenderPresent(renderer);
	}

	SDL_HapticClose(haptic);
	SDL_DestroyTexture(bg);
	// todo: destroy other textures
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}

bullet* spawn_bullets(spaceship* ship, int velocity, int spread) {
	

	double spread_angle = 3.14159 * 0.4368789353;

	bullet* new_bullets = (bullet*)malloc(sizeof(bullet) * spread); // todo: free me

	if (spread % 2 == 0) {

	}
	else {
		double start_angle = spread_angle * (spread - 1) / 2;
		for (int i = 0; i < spread; i++) {
			int extra_x_vel = velocity*ship->gun_dir_x / sqrt(pow(ship->gun_dir_x, 2) + pow(ship->gun_dir_y, 2));
			int extra_y_vel = velocity*ship->gun_dir_y / sqrt(pow(ship->gun_dir_x, 2) + pow(ship->gun_dir_y, 2));

			new_bullets[i] = init_bullet(ship->x_pos, ship->y_pos, ship->x_vel + extra_x_vel, ship->y_vel + extra_y_vel)[0];
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
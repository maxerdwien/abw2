#include <iostream>
#include <string>

#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>


#include "spaceship.h"
#include "grizzly.h"
#include "black.h"

#include "renderer.h"

SDL_Renderer* renderer;
TTF_Font* caladea36;

SDL_Window* window;

int WINDOW_WIDTH;
int WINDOW_HEIGHT;
const int BARSIZE = 26;
const int STATUS_BAR_WIDTH = 150;

bool quit = false;
bool is_fullscreen = false;
bool xp_mode = false;

int controller_mappings[4] = { -1, -1, -1, -1 };
SDL_GameController* controllers[4] = { NULL, NULL, NULL, NULL };
SDL_Haptic* haptics[4];

bool read_global_input(SDL_Event* e);

enum ship_type {
	black = 0,
	grizzly = 1,
	polar = 2
};

void render_character_selector(int x, int y, SDL_Texture* ship_tex, ship_type shipType,  SDL_Texture* right_arrow, SDL_Texture* left_arrow);
void render_plugin_to_join(int x, int y);
int lookup_controller(int instanceID);

int main(int, char**) {
	int test = TTF_Init(); // todo: delete or rename

	enum gameState {
		mainMenu,
		options,
		characterSelect,
		stageSelect,
		initGame,
		inGame,
		results,
		pause
	};

	enum color {
		red = 0,
		blue = 1,
		yellow = 2,
		green = 3
	};



	gameState currentState = mainMenu;
	//gameState currentState = inGame;

	// init SDL
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0) {
		std::cout << "SDL_Init error: " << SDL_GetError() << std::endl;
		return 1;
	}

	// set resolution
	enum Resolution {
		_480p,
		_720p,
		_1080p,
		_1440p,
		_2160p // 4k
	};
	const Resolution res = _720p;
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
	case _1440p:
		WINDOW_WIDTH = 2560;
		WINDOW_HEIGHT = 1440;
		break;
	case _2160p:
		WINDOW_WIDTH = 3840;
		WINDOW_HEIGHT = 2160;
		break;
	default:
		WINDOW_WIDTH = 1280;
		WINDOW_HEIGHT = 720;
	}

	const double CONTROLLER_MAX_ANGLE = M_PI / 6;
	const int DEAD_ZONE = 5000;
	
	const int playerUiWidth = 10;

	window = SDL_CreateWindow("hl2.exe", 675, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (!window) {
		std::cout << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		SDL_DestroyWindow(window);
		std::cout << "SDL_CreateRenderer error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	// register controllers
	for (int i = 0; i < 4; i++) {
		controllers[i] = NULL;
	}

	// load textures
	SDL_Texture* bg = LoadTexture("..\\Project1\\assets\\background.png");
	//SDL_Texture* sun_tex = LoadTexture("..\\Project1\\assets\\sun.png");



	ship_type selections[4] = { grizzly, grizzly, grizzly, grizzly };
	bool analog_stick_moved[4] = { false, false, false, false };

	SDL_Texture* ship_textures[3][4];
	ship_textures[black][red] = LoadTexture("..\\Project1\\assets\\ships\\black-red.png");
	ship_textures[black][blue] = LoadTexture("..\\Project1\\assets\\ships\\black-blue.png");
	ship_textures[black][yellow] = LoadTexture("..\\Project1\\assets\\ships\\black-yellow.png");
	ship_textures[black][green] = LoadTexture("..\\Project1\\assets\\ships\\black-green.png");

	ship_textures[grizzly][red] = LoadTexture("..\\Project1\\assets\\ships\\grizzly-red.png");
	ship_textures[grizzly][blue] = LoadTexture("..\\Project1\\assets\\ships\\grizzly-blue.png");
	ship_textures[grizzly][yellow] = LoadTexture("..\\Project1\\assets\\ships\\grizzly-yellow.png");
	ship_textures[grizzly][green] = LoadTexture("..\\Project1\\assets\\ships\\grizzly-green.png");

	ship_textures[polar][red] = LoadTexture("..\\Project1\\assets\\ships\\polar-red.png");
	ship_textures[polar][blue] = LoadTexture("..\\Project1\\assets\\ships\\polar-blue.png");
	ship_textures[polar][yellow] = LoadTexture("..\\Project1\\assets\\ships\\polar-yellow.png");
	ship_textures[polar][green] = LoadTexture("..\\Project1\\assets\\ships\\polar-green.png");
	
	SDL_Texture* right_arrow = LoadTexture("..\\Project1\\assets\\right_arrow.png");
	SDL_Texture* left_arrow = LoadTexture("..\\Project1\\assets\\left_arrow.png");

	caladea36 = TTF_OpenFont("..\\Project1\\assets\\caladea-regular.ttf", 36); //this opens a font style and sets a size

	SDL_Event e;
	
	int winner;

	Ship* ships[4];

	Uint32 last_frame_start_time = SDL_GetTicks();
	Uint32 frame_start_time = SDL_GetTicks();

	const int frame_counter_size = 60;
	int frame_time[frame_counter_size + 1];
	int frame_counter = 0;

	
	if (is_fullscreen) {
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
		SDL_ShowCursor(0);
	}

	// game loop
	while (!quit) {
		last_frame_start_time = frame_start_time;
		frame_start_time = SDL_GetTicks();
		frame_time[frame_counter] = frame_start_time - last_frame_start_time;
		frame_counter++;
		if (frame_counter > frame_counter_size) {
			frame_counter = 0;
			int total_time = 0;
			for (int i = 0; i < frame_counter_size; i++) {
				total_time += frame_time[i];
			}
			double total_seconds = (double)total_time * 60.0 / 1000;
			//std::cout << total_seconds << " fps" << std::endl;
		}

		// start of main menu state
		if (currentState == mainMenu) {
			while (SDL_PollEvent(&e)) {
				if (read_global_input(&e)) continue;
				switch (e.type) {
				case SDL_CONTROLLERBUTTONDOWN:
					if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
						currentState = characterSelect;
					} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
						quit = true;
					}
					break;
				}
			}

			// render basic black background
			{
				SDL_SetRenderDrawColor(renderer, 00, 00, 00, SDL_ALPHA_OPAQUE);
				SDL_Rect s;
				s.x = 0;
				s.y = 0;
				s.w = WINDOW_WIDTH;
				s.h = WINDOW_HEIGHT;
				SDL_RenderFillRect(renderer, &s);
			}

			// render title name and prompt to move forward
			{
				render_text_centered(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2.5, "Alaskan Cosmobear Spacefighting");

				render_text_centered(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, "Press the A button to start.");

				render_text_centered(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 1.7, "Press the B button to quit.");
			}

			SDL_RenderPresent(renderer);

		}
		// start of character select state
		else if (currentState == characterSelect) {

			int controller_index;

			while (SDL_PollEvent(&e)) {
				if (read_global_input(&e)) continue;
				switch (e.type) {
				case SDL_CONTROLLERBUTTONDOWN:
					controller_index = lookup_controller(e.cbutton.which);
					
					if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
						currentState = stageSelect;
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
						currentState = mainMenu;
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
						switch (selections[controller_index]) {
						case black:
							selections[controller_index] = grizzly;
							break;
						case grizzly:
							selections[controller_index] = polar;
							break;
						case polar:
							selections[controller_index] = black;
							break;
						}
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
						switch (selections[controller_index]) {
						case black:
							selections[controller_index] = polar;
							break;
						case grizzly:
							selections[controller_index] = black;
							break;
						case polar:
							selections[controller_index] = grizzly;
							break;
						}
					} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
						
					}
					break;
				case SDL_CONTROLLERAXISMOTION:
					controller_index = lookup_controller(e.caxis.which);
					if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
						int min_angle = 26000;

						if (e.caxis.value > min_angle && !analog_stick_moved[controller_index]) {
							analog_stick_moved[controller_index] = true;
							switch (selections[controller_index]) {
							case black:
								selections[controller_index] = polar;
								break;
							case grizzly:
								selections[controller_index] = black;
								break;
							case polar:
								selections[controller_index] = grizzly;
								break;
							}
						} else if (e.caxis.value < -min_angle && !analog_stick_moved[controller_index]) {
							analog_stick_moved[controller_index] = true;
							switch (selections[controller_index]) {
							case black:
								selections[controller_index] = grizzly;
								break;
							case grizzly:
								selections[controller_index] = polar;
								break;
							case polar:
								selections[controller_index] = black;
								break;
							}
						} else if (e.caxis.value < min_angle && e.caxis.value > -min_angle) {
							analog_stick_moved[controller_index] = false;
						}
					}
					break;
				}
			}

			// todo: remove this?
			SDL_RenderClear(renderer);

			// render basic black background
			{
				SDL_SetRenderDrawColor(renderer, 00, 00, 00, SDL_ALPHA_OPAQUE);
				SDL_Rect s;
				s.x = 0;
				s.y = 0;
				s.w = WINDOW_WIDTH;
				s.h = WINDOW_HEIGHT;
				SDL_RenderFillRect(renderer, &s);
			}

			// render character select dividers vertical
			{
				SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
				SDL_Rect s;
				s.x = WINDOW_WIDTH/2 - BARSIZE/2;
				s.y = 0;
				s.w = BARSIZE;
				s.h = WINDOW_HEIGHT;
				SDL_RenderFillRect(renderer, &s);
			}

			// render character select dividers horizontal
			{
				SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
				SDL_Rect s;
				s.x = 0;
				s.y = WINDOW_HEIGHT / 2 - BARSIZE/2;
				s.w = WINDOW_WIDTH;
				s.h = BARSIZE;
				SDL_RenderFillRect(renderer, &s);
			}
			
			// render ships selections
			{
				if (controllers[0]) {
					render_character_selector(0, 0, ship_textures[selections[0]][red],selections[0],right_arrow, left_arrow);
				} else {
					render_plugin_to_join(0, 0);
				}
				if (controllers[1]) {
					render_character_selector(WINDOW_WIDTH / 2 + BARSIZE/2, 0, ship_textures[selections[1]][blue],selections[1],right_arrow, left_arrow);
				} else {
					render_plugin_to_join(WINDOW_WIDTH / 2 + BARSIZE/2, 0);
				}
				if (controllers[2]) {
					render_character_selector(0, WINDOW_HEIGHT / 2 + BARSIZE, ship_textures[selections[2]][yellow],selections[2], right_arrow, left_arrow);
				} else {
					render_plugin_to_join(0, WINDOW_HEIGHT / 2 + BARSIZE/2);
				}
				if (controllers[3]) {
					render_character_selector(WINDOW_WIDTH / 2 + BARSIZE, WINDOW_HEIGHT / 2 + BARSIZE, ship_textures[selections[3]][green],selections[3], right_arrow, left_arrow);
				} else {
					render_plugin_to_join(WINDOW_WIDTH / 2 + BARSIZE/2, WINDOW_HEIGHT / 2 + BARSIZE/2);
				}
			}

			SDL_RenderPresent(renderer);

		}
		// start of stage select state
		else if (currentState == stageSelect) {
			currentState = initGame;
			
		}
		else if (currentState == initGame) {
			// init game objects
			const int spawn_locations_x[4] = {
				10000 * ((WINDOW_WIDTH - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH) ,
				10000 * (3 * (WINDOW_WIDTH - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH),
				10000 * (2 * (WINDOW_WIDTH - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH),
				10000 * (2 * (WINDOW_WIDTH - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH)
			};
			const int spawn_locations_y[4] = {
				10000 * (WINDOW_HEIGHT / 2),
				10000 * (WINDOW_HEIGHT / 2),
				10000 * (WINDOW_HEIGHT / 4),
				10000 * (3 * WINDOW_HEIGHT / 4)
			};
			for (int i = 0; i < 4; i++) {
				switch (selections[i]) {
				case black:
					ships[i] = new Black(i, spawn_locations_x[i], spawn_locations_y[i]);
					break;
				case grizzly:
					ships[i] = new Grizzly(i, spawn_locations_x[i], spawn_locations_y[i]);
					break;
				case polar:
					// todo: do the obvious thing
					ships[i] = new Grizzly(i, spawn_locations_x[i], spawn_locations_y[i]);
					break;
				}
				if (!controllers[i]) {
					ships[i] = NULL;
				}
			}

			// render background once; this is needed for xp mode
			SDL_Rect r;
			r.x = 0;
			r.y = 0;
			r.w = WINDOW_WIDTH;
			r.h = WINDOW_HEIGHT;
			RenderCopyEx(bg, NULL, &r, 0, NULL, SDL_FLIP_NONE);

			currentState = inGame;
			
		}
		// start of in game state
		else if (currentState == inGame) {
			// poll input
			{
				int controller_index;
				Ship* ship;
				while (SDL_PollEvent(&e)) {
					if (read_global_input(&e)) continue;
					switch (e.type) {
					case SDL_CONTROLLERBUTTONDOWN:
						controller_index = lookup_controller(e.cbutton.which);
						ship = ships[controller_index];
						if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_X) {

						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_Y) {

						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
							ship->do_fire_1 = true;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
							ship->do_fire_3 = true;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSTICK) {
							ship->do_speed_boost = true;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
							currentState = pause;
						}
						break;
					case SDL_CONTROLLERBUTTONUP:
						controller_index = lookup_controller(e.cbutton.which);
						ship = ships[controller_index];
						if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
							ship->do_fire_1 = false;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
							ship->do_fire_3 = false;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSTICK) {
							ship->do_speed_boost = false;
						}
						break;
					case SDL_CONTROLLERAXISMOTION:
						controller_index = lookup_controller(e.caxis.which);
						ship = ships[controller_index];
						if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
							double value = (double)e.caxis.value / 32767;

							double circle_x = sin(value * CONTROLLER_MAX_ANGLE) / sin(CONTROLLER_MAX_ANGLE);
							//std::cout << old_gun_mag << std::endl;
							ship->left_stick_x = (int)(10000 * circle_x);
							double left_stick_mag = sqrt(pow(ship->left_stick_x, 2) + pow(ship->left_stick_y, 2));
							std::cout << left_stick_mag << std::endl;
							if (left_stick_mag > DEAD_ZONE) {
								ship->move_dir_x = ship->left_stick_x;
								ship->face_dir_x = ship->left_stick_x;
								ship->move_dir_y = ship->left_stick_y;
								ship->face_dir_y = ship->left_stick_y;
							} else {
								ship->move_dir_x = 0;
								ship->move_dir_y = 0;
							}
						} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
							double value = (double)e.caxis.value / 32768;

							double circle_y = sin(value * CONTROLLER_MAX_ANGLE) / sin(CONTROLLER_MAX_ANGLE);
							//std::cout << old_gun_mag << std::endl;
							ship->left_stick_y = (int)(10000 * circle_y);
							double left_stick_mag = sqrt(pow(ship->left_stick_x, 2) + pow(ship->left_stick_y, 2));
							//std::cout << left_stick_mag << std::endl;
							if (left_stick_mag > DEAD_ZONE) {
								ship->move_dir_x = ship->left_stick_x;
								ship->face_dir_x = ship->left_stick_x;
								ship->move_dir_y = ship->left_stick_y;
								ship->face_dir_y = ship->left_stick_y;
							} else {
								ship->move_dir_x = 0;
								ship->move_dir_y = 0;
							}
						} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
							double value = (double)e.caxis.value / 32767;

							double circle_x = sin(value * CONTROLLER_MAX_ANGLE) / sin(CONTROLLER_MAX_ANGLE);
							ship->right_stick_x = (int)(10000 * circle_x);
							double right_stick_mag = sqrt(pow(ship->right_stick_x, 2) + pow(ship->right_stick_y, 2));
							//std::cout << right_stick_mag << std::endl;
							if (right_stick_mag > DEAD_ZONE) {
								ship->gun_dir_x = ship->right_stick_x;
								ship->gun_dir_y = ship->right_stick_y;
							}

						} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
							double value = (double)e.caxis.value / 32768;

							double circle_y = sin(value * CONTROLLER_MAX_ANGLE) / sin(CONTROLLER_MAX_ANGLE);
							ship->right_stick_y = (int)(10000 * circle_y);
							double right_stick_mag = sqrt(pow(ship->right_stick_x, 2) + pow(ship->right_stick_y, 2));
							//std::cout << right_stick_mag << std::endl;
							if (right_stick_mag > DEAD_ZONE) {
								ship->gun_dir_x = ship->right_stick_x;
								ship->gun_dir_y = ship->right_stick_y;
							}

						} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
							int min_activation = 20000;
							//std::cout << e.caxis.value << std::endl;
							if (e.caxis.value < min_activation) {
								ship->do_fire_2 = false;
							} else {
								ship->do_fire_2 = true;
							}
						} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
							int min_activation = 20000;
							//std::cout << e.caxis.value << std::endl;
							if (e.caxis.value < min_activation) {
								ship->do_fire_2 = false;
							} else {
								ship->do_fire_2 = true;
							}
						}
						break;
					}
				}
			}

			if (currentState == pause) {
				continue;
			}

			// update
			for (int i = 0; i < 4; i++) {
				if (!ships[i]) continue;
				Ship* ship = ships[i];

				// regen stamina
				ship->stamina += ship->stamina_per_frame;
				if (ship->stamina > ship->stamina_max) {
					ship->stamina = ship->stamina_max;
				}

				// reduce invincibility time
				if (ship->invincibility_cooldown > 0) {
					ship->invincibility_cooldown--;
				}

				// handle projectile spawns
				ship->fire_1();

				ship->fire_2();

				ship->fire_3();

				/*
				

				// handle spread fire spawns
				if (ship->spread_cooldown > 0) {
					ship->spread_cooldown--;
				}
				if (ship->fire_spread && ship->stamina > 0 && ship->spread_cooldown <= 0) {

					int MUZZLE_VEL = 40000;
					int spread = 5;
					bullet** new_bullets = spawn_bullets(ship, MUZZLE_VEL, spread, 5, 10, 100);
					for (int i = 0; i < spread; i++) {
						ship->bullets[ship->num_bullets] = new_bullets[i];
						ship->num_bullets++;
					}
					free(new_bullets);
					ship->spread_cooldown += ship->spread_delay;
					ship->stamina -= 250;
				}
				*/

				// update ship itself
				{

					if (ship->speed_boost_cooldown > 0) {
						ship->speed_boost_cooldown--;
					}

					// update acceleration
					double accel_mag = sqrt(pow(ship->move_dir_x, 2) + pow(ship->move_dir_y, 2));
					if (accel_mag > ship->max_accel) {
						accel_mag = ship->max_accel;
					}
					if (ship->do_speed_boost && ship->speed_boost_cooldown == 0) {
						accel_mag = ship->max_accel * 40;
						ship->speed_boost_cooldown += ship->speed_boost_delay;
						ship->do_speed_boost = false;
						ship->stamina -= 200;
					}

					if (ship->move_dir_x != 0 || ship->move_dir_y != 0) {
						ship->x_accel = (int)(accel_mag*ship->move_dir_x / sqrt(pow(ship->move_dir_x, 2) + pow(ship->move_dir_y, 2)));
						ship->y_accel = (int)(accel_mag*ship->move_dir_y / sqrt(pow(ship->move_dir_x, 2) + pow(ship->move_dir_y, 2)));
					} else {
						ship->x_accel = 0;
						ship->y_accel = 0;
					}

					ship->x_vel += ship->x_accel;
					ship->y_vel += ship->y_accel;

					// friction
					int iterations_per_frame = 40;
					for (int j = 0; j < iterations_per_frame; j++) {
						double total_vel = sqrt(pow(ship->x_vel, 2) + pow(ship->y_vel, 2));
						//double friction_accel = ((pow(total_vel, 2) + (double)ship->constant_friction) / (double)ship->friction_limiter) / iterations_per_frame;
						// todo: make this use the constants instead of literals
						// todo: scale linearly with velocity?
						double friction_accel = ((pow(total_vel, 2) + 1500000000) / 2000000) / iterations_per_frame;

						int friction_accel_x = ((double)friction_accel * ship->x_vel) / total_vel;
						if (ship->x_vel > 0) {
							ship->x_vel -= friction_accel_x;
							if (ship->x_vel < 0) {
								ship->x_vel = 0;
							}
						} else if (ship->x_vel < 0) {
							ship->x_vel -= friction_accel_x;
							if (ship->x_vel > 0) {
								ship->x_vel = 0;
							}
						}

						int friction_accel_y = ((double)friction_accel * ship->y_vel) / total_vel;
						if (ship->y_vel > 0) {
							ship->y_vel -= friction_accel_y;
							if (ship->y_vel < 0) {
								ship->y_vel = 0;
							}
						} else if (ship->y_vel < 0) {
							ship->y_vel -= friction_accel_y;
							if (ship->y_vel > 0) {
								ship->y_vel = 0;
							}
						}
					}

					// handle collisions between ships
					for (int j = 0; j < 4; j++) {
						if (!ships[j]) continue;
						if (i == j) continue;
						double dist = sqrt(pow(ship->x_pos - ships[j]->x_pos, 2) + pow(ship->y_pos - ships[j]->y_pos, 2));
						if (dist == 0) dist = 1;
						if (dist <= 10000*(ships[i]->radius + ships[j]->radius)) {
							double total_force = 140000000000000000.0 / pow(dist, 2);
							double x_force = (ship->x_pos - ships[j]->x_pos) * total_force / dist;
							ship->x_vel += x_force/ship->weight;
							double y_force = (ship->y_pos - ships[j]->y_pos) * total_force / dist;
							ship->y_vel += y_force/ship->weight;
						}
					}

					// update position
					ship->x_pos += ship->x_vel;
					ship->y_pos += ship->y_vel;

					// handle death
					if (ship->x_pos < STATUS_BAR_WIDTH * 10000 || ship->x_pos > WINDOW_WIDTH * 10000 || ship->y_pos < 0 || ship->y_pos > WINDOW_HEIGHT * 10000) {
						
						ship->lives--;
						SDL_HapticRumblePlay(haptics[i], 1, 300);

						if (ship->lives <= 0) {
							ship->lives = 0;
							// todo: something
						}

						ship->invincibility_cooldown += ship->respawn_invincibility_delay;

						ship->x_pos = ship->respawn_x;
						ship->y_pos = ship->respawn_y;
						ship->x_vel = 0;
						ship->y_vel = 0;

						ship->percent = 0;
					}

				}

				// update projectiles
				ship->update_projectiles_1(STATUS_BAR_WIDTH * 10000, WINDOW_WIDTH * 10000, 0, WINDOW_HEIGHT * 10000, ships, haptics);
				ship->update_projectiles_2(STATUS_BAR_WIDTH * 10000, WINDOW_WIDTH * 10000, 0, WINDOW_HEIGHT * 10000, ships, haptics);
				ship->update_projectiles_3(STATUS_BAR_WIDTH * 10000, WINDOW_WIDTH * 10000, 0, WINDOW_HEIGHT * 10000, ships, haptics);

				// Check to see if the game is over
				int number_alive = 0;
				winner = -1;
				for (int i = 0; i < 4; i++) {
					if (!ships[i]) continue;
					if (ships[i]->lives != 0) {
						number_alive++;
						winner = i+1;
					}
				}
				if (number_alive == 1) {
					currentState = results;
				}

			} // end of update ships

			// begin rendering
			{
				// render background
				if (!xp_mode) {
					SDL_Rect r;
					r.x = 0;
					r.y = 0;
					r.w = WINDOW_WIDTH;
					r.h = WINDOW_HEIGHT;
					RenderCopyEx(bg, NULL, &r, 0, NULL, SDL_FLIP_NONE);
				}

				// render all ship elements
				for (int i = 0; i < 4; i++) {
					if (!ships[i]) continue;
					Ship* ship = ships[i];

					ship->render();

					// render projectiles
					ship->render_projectiles_1();
					ship->render_projectiles_2();
					ship->render_projectiles_3();

				}

				// render UI elements
				{
					// render status bar background
					SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
					SDL_Rect s;
					s.x = 0;
					s.y = 0;
					s.w = STATUS_BAR_WIDTH;
					s.h = WINDOW_HEIGHT;
					SDL_RenderFillRect(renderer, &s);

					int textAdjustment = 50; // todo: rename this shit

					// render each ship's UI elements
					for (int i = 0; i < 4; i++) {
						if (!ships[i]) continue;
						Ship* ship = ships[i];
						
						// render stamina bar
						{
							// todo: match these to the real colors
							// set stamina bar color
							if (ship->id == 0) {
								SDL_SetRenderDrawColor(renderer, 160, 0, 0, SDL_ALPHA_OPAQUE);
							} else if (ship->id == 1) {
								SDL_SetRenderDrawColor(renderer, 0, 0, 160, SDL_ALPHA_OPAQUE);
							} else if (ship->id == 2) {
								SDL_SetRenderDrawColor(renderer, 210, 210, 0, SDL_ALPHA_OPAQUE);
							} else {
								SDL_SetRenderDrawColor(renderer, 0, 160, 0, SDL_ALPHA_OPAQUE);
							}

							SDL_Rect r;
							r.x = 0;
							r.y = 120 + 100 * i + (textAdjustment * i);
							r.w = 150 * ship->stamina / ship->stamina_max;
							r.h = 30;
							SDL_RenderFillRect(renderer, &r);
						}

						// render percentages
						{
							char str[10];
							snprintf(str, 10, "P%d: %d%%", i + 1, ship->percent);
							render_text(0, (60 + 100 * i + 1) + textAdjustment * i, str);
						}

						// render stock counter
						{
							char playerLives[20];
							sprintf_s(playerLives, "Lives: %d", ship->lives);
							render_text(0, (10 + 150 * i), playerLives);
						}
					}
				}
				SDL_RenderPresent(renderer);
			}
		} //end of ingame state

		// start pause state
		else if (currentState == pause) {

			while (SDL_PollEvent(&e)) {
				if (read_global_input(&e)) continue;
				switch (e.type) {
				case SDL_CONTROLLERBUTTONDOWN:
					if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
						currentState = inGame;
					}
					break;
				}
			}

			// render pause background
			{
				SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
				SDL_Rect s;
				s.x = WINDOW_WIDTH / 2;
				s.y = WINDOW_HEIGHT / 2;
				s.w = 120;
				s.h = 50;
				SDL_RenderFillRect(renderer, &s);
			}

			// render word "paused"
			render_text(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, "Paused");

			SDL_RenderPresent(renderer);


		} //end pause state

		// stage select state
		else if (currentState == stageSelect) {

			currentState = inGame;
		}
		// start of options state
		else if (currentState == options) {

		}
		// start of results state
		else if (currentState == results) {
			while (SDL_PollEvent(&e)) {
				if (read_global_input(&e)) continue;
				switch (e.type) {
				case SDL_CONTROLLERBUTTONDOWN:
					if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
						currentState = mainMenu;
					}
					break;
				}
			}
			// todo: remove?
			SDL_RenderClear(renderer);

			// render basic black background
			{
				SDL_SetRenderDrawColor(renderer, 00, 00, 00, SDL_ALPHA_OPAQUE);
				SDL_Rect s;
				s.x = 0;
				s.y = 0;
				s.w = WINDOW_WIDTH;
				s.h = WINDOW_HEIGHT;
				SDL_RenderFillRect(renderer, &s);
			}

			// render who won plus continue prompt
			{
				char winnerMessage[15];
				sprintf_s(winnerMessage, "Player %d wins!", winner);
				render_text_centered(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2.5, winnerMessage);

				render_text_centered(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, "Press the A button to continue.");
			}
			SDL_RenderPresent(renderer);
		}

		
	} //end of loop

	//for (int i = 0; i < num_players; i++) {
		// todo: close the controllers themselves?
	//	SDL_HapticClose(haptics[i]);
	//}
	SDL_DestroyTexture(bg);
	// todo: destroy other textures?
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}


bool read_global_input(SDL_Event* e) {
	bool event_eaten = false;
	SDL_Keycode k;
	switch (e->type) {
	case SDL_QUIT:
		quit = true;
		event_eaten = true;
		break;
	case SDL_KEYDOWN:
		k = e->key.keysym.sym;
		if (k == SDLK_f) {
			if (is_fullscreen) {
				SDL_SetWindowFullscreen(window, 0);
				SDL_ShowCursor(1);
			} else {
				SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
				SDL_ShowCursor(0);
			}
			is_fullscreen = !is_fullscreen;
			event_eaten = true;
		} else if (k == SDLK_ESCAPE) {
			if (is_fullscreen) {
				SDL_SetWindowFullscreen(window, 0);
				is_fullscreen = !is_fullscreen;
				SDL_ShowCursor(1);
			}
			event_eaten = true;
		}
		break;
	case SDL_CONTROLLERBUTTONDOWN:
		if (e->cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
			quit = true;
			event_eaten = true;
		} else if (e->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
			xp_mode = !xp_mode;
		}
		break;
	case SDL_CONTROLLERDEVICEADDED:
	{
		SDL_GameController* temp = SDL_GameControllerOpen(e->cdevice.which);
		if (!temp) {
			std::cout << "could not connect controller" << std::endl;
		}
		SDL_Joystick* joy = SDL_GameControllerGetJoystick(temp);
		int instanceID = SDL_JoystickInstanceID(joy);
		// find first empty slot
		for (int i = 0; i < 4; i++) {
			if (controller_mappings[i] == -1) {
				controller_mappings[i] = instanceID;
				controllers[i] = temp;

				haptics[i] = SDL_HapticOpen(e->cdevice.which);
				if (SDL_HapticRumbleInit(haptics[i]) != 0) {
					std::cout << "could not init haptic device" << std::endl;
				}
				SDL_HapticRumblePlay(haptics[i], 1.0f, 500);
				if (!haptics[i]) {
					printf(SDL_GetError());
				}
				std::cout << "connecting new controller " << instanceID << " in slot " << i << std::endl;
				break;
			}
		}
		event_eaten = true;
		break;
	}
	case SDL_CONTROLLERDEVICEREMOVED:
	{
		int instanceID = e->cdevice.which;
		int slot = lookup_controller(instanceID);
		if (slot == -1) printf("something went terribly wrong for controller with instanceID %d", instanceID);
		std::cout << "disconnecting controller " << instanceID << " from slot " << slot << std::endl;
		controller_mappings[slot] = -1;
		SDL_GameControllerClose(controllers[slot]);
		controllers[slot] = NULL;
		SDL_HapticClose(haptics[slot]);
		haptics[slot] = NULL;
		event_eaten = true;
		break;
	}
	}
	return event_eaten;
}

void render_character_selector(int x, int y, SDL_Texture* ship_tex,  ship_type shipType, SDL_Texture* right_arrow, SDL_Texture* left_arrow) {
	render_texture(ship_tex, x+300, y+70, 0, 4);
	render_texture(right_arrow, x+400, y+70,0, 1);
	render_texture(left_arrow, x+200, y+70, 0, 1);
	if (shipType == 0) {
		render_text_centered(x + 300, y + 125, "BLACK");
		render_text(x + 100, y + 155, "Weapon 1: Burst Shot");
		render_text(x + 100, y + 185, "Weapon 2: Melee Weapon");
		render_text(x + 100, y + 215, "Weapon 3: Charge Shot");
	}
	else if (shipType == 1) {
		render_text_centered(x + 300, y + 125, "GRIZZLY");
		render_text(x + 100, y + 155, "Weapon 1: Normal Shot");
		render_text(x + 100, y + 185, "Weapon 2: Normal Missles");
		render_text(x + 100, y + 215, "Weapon 3: Mines");
	}
	else {
		render_text_centered(x + 300, y + 125, "POLAR");
		render_text(x + 100, y + 155, "Weapon 1: Spread Shot");
		render_text(x + 100, y + 185, "Weapon 2: Gravity Missles");
		render_text(x + 100, y + 215, "Weapon 3: Laser");
	}
}

void render_plugin_to_join(int x, int y) {
	render_text(x+100, y+100, "Plug in controller to join");
}


int lookup_controller(int instanceID) {
	for (int i = 0; i < 4; i++) {
		if (controller_mappings[i] == instanceID) {
			return i;
		}
	}
	return -1;
}
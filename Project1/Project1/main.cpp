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

SDL_Window* window;

int WINDOW_WIDTH;
int WINDOW_HEIGHT;
const int STATUS_BAR_WIDTH = 150;

bool quit = false;
bool is_fullscreen = false;

bool read_global_input(SDL_Event* e);
void render_character_selector(int x, int y, SDL_Texture* ship_tex, SDL_Texture* right_arrow, SDL_Texture* left_arrow);

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


	const int DEAD_ZONE = 5000;
	
	const int playerUiWidth = 10;

	window = SDL_CreateWindow("hl2.exe", 1000, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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
	SDL_GameController* controllers[4];
	SDL_Haptic* haptics[4];

	const int num_players = 2;
	//const int num_players = SDL_NumJoysticks();
	std::cout << "controllers detected: " << num_players << std::endl;
	for (int i = 0; i < num_players; i++) {
		controllers[i] = NULL;
		{
			if (SDL_IsGameController(i)) {
				controllers[i] = SDL_GameControllerOpen(i);
				// todo: fix this hack
				haptics[i] = SDL_HapticOpen((i + 1) % 2);
				{
					if (!haptics[i]) {
						std::cout << "could not connect haptic device" << std::endl;
					}
					if (SDL_HapticRumbleInit(haptics[i]) != 0) {
						std::cout << "could not init haptic device" << std::endl;
					}
				}
			}
			if (!controllers[i]) {
				std::cout << "could not connect controller " << i << std::endl;
			}
		}

	}

	// load textures
	SDL_Texture* bg = LoadTexture("..\\Project1\\assets\\background.png");
	SDL_Texture* sun_tex = LoadTexture("..\\Project1\\assets\\sun.png");

	enum color {
		red = 0,
		blue = 1,
		yellow = 2,
		green = 3
	};
	enum ship_type {
		black = 0,
		grizzly = 1,
		polar = 2
	};

	ship_type selections[4] = { grizzly, grizzly, grizzly, grizzly };

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

	TTF_Font* caladea36 = TTF_OpenFont("..\\Project1\\assets\\caladea-regular.ttf", 36); //this opens a font style and sets a size

	SDL_Event e;
	
	int winner;

	Ship* ships[2];

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
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
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
				SDL_Color White = { 255, 255, 255 };  // Renders the color of the text
				SDL_Surface* titleSurface = TTF_RenderText_Blended(caladea36, "Alaskan Cosmobear Spacefighting", White); //Create the sdl surface
				SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface); //Convert to texture
				SDL_Rect titleRect; //create a rect
				titleRect.x = WINDOW_WIDTH/3.5;
				titleRect.y = WINDOW_HEIGHT/2.5;
				SDL_QueryTexture(titleTexture, NULL, NULL, &titleRect.w, &titleRect.h);
				SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
				SDL_DestroyTexture(titleTexture);
				SDL_FreeSurface(titleSurface);

				SDL_Surface* promptSurface = TTF_RenderText_Blended(caladea36, "Press the A button to start.", White); //Create the sdl surface
				SDL_Texture* promptTexture = SDL_CreateTextureFromSurface(renderer, promptSurface); //Convert to texture
				SDL_Rect promptRect; //create a rect
				promptRect.x = WINDOW_WIDTH / 3.5;
				promptRect.y = WINDOW_HEIGHT / 2;
				SDL_QueryTexture(promptTexture, NULL, NULL, &promptRect.w, &promptRect.h);
				SDL_RenderCopy(renderer, promptTexture, NULL, &promptRect);
				SDL_DestroyTexture(promptTexture);
				SDL_FreeSurface(promptSurface);

				SDL_Surface* quitSurface = TTF_RenderText_Blended(caladea36, "Press the B button to quit.", White); //Create the sdl surface
				SDL_Texture* quitTexture = SDL_CreateTextureFromSurface(renderer, quitSurface); //Convert to texture
				SDL_Rect quitRect; //create a rect
				quitRect.x = WINDOW_WIDTH / 3.5;
				quitRect.y = WINDOW_HEIGHT / 1.7;
				SDL_QueryTexture(quitTexture, NULL, NULL, &quitRect.w, &quitRect.h);
				SDL_RenderCopy(renderer, quitTexture, NULL, &quitRect);
				SDL_DestroyTexture(quitTexture);
				SDL_FreeSurface(quitSurface);
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
					controller_index = e.cbutton.which;
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
				s.x = WINDOW_WIDTH/2;
				s.y = 0;
				s.w = 25;
				s.h = WINDOW_HEIGHT;
				SDL_RenderFillRect(renderer, &s);
			}

			// render character select dividers horizontal
			{
				SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
				SDL_Rect s;
				s.x = 0;
				s.y = WINDOW_HEIGHT / 2;
				s.w = WINDOW_WIDTH;
				s.h = 25;
				SDL_RenderFillRect(renderer, &s);
			}
			
			// render ships selections
			{
				render_character_selector(0, 0,								ship_textures[selections[0]][red], right_arrow, left_arrow);
				render_character_selector(WINDOW_WIDTH/2, 0,				ship_textures[selections[1]][blue], right_arrow, left_arrow);
				render_character_selector(0, WINDOW_HEIGHT/2,				ship_textures[selections[2]][yellow], right_arrow, left_arrow);
				render_character_selector(WINDOW_WIDTH/2, WINDOW_HEIGHT/2,	ship_textures[selections[3]][green], right_arrow, left_arrow);
			}

			SDL_RenderPresent(renderer);

		}
		// start of stage select state
		else if (currentState == stageSelect) {
			currentState = initGame;
			
		}
		else if (currentState == initGame) {
			// init game objects
			switch(selections[0]) {
			case black:
				ships[0] = new Black(0, 10000 * ((WINDOW_WIDTH - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH), 10000 * WINDOW_HEIGHT / 2);
				break;
			case grizzly:
				ships[0] = new Grizzly(0, 10000 * ((WINDOW_WIDTH - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH), 10000 * WINDOW_HEIGHT / 2);
				break;
			case polar:
				// todo: do the obvious thing
				ships[0] = new Grizzly(0, 10000 * ((WINDOW_WIDTH - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH), 10000 * WINDOW_HEIGHT / 2);
				break;
			}
			switch (selections[1]) {
			case black:
				ships[1] = new Black(1, 10000 * (3 * (WINDOW_WIDTH - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH), 10000 * WINDOW_HEIGHT / 2);
				break;
			case grizzly:
				ships[1] = new Grizzly(1, 10000 * (3 * (WINDOW_WIDTH - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH), 10000 * WINDOW_HEIGHT / 2);
				break;
			case polar:
				// todo: do the obvious thing
				ships[1] = new Grizzly(1, 10000 * (3 * (WINDOW_WIDTH - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH), 10000 * WINDOW_HEIGHT / 2);
				break;
			}

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
						controller_index = e.cbutton.which;
						ship = ships[controller_index];
						if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_X) {

						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_Y) {

						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
							ship->do_fire_1 = true;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
							ship->do_fire_3 = true;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
							currentState = pause;
						}
						break;
					case SDL_CONTROLLERBUTTONUP:
						controller_index = e.cbutton.which;
						ship = ships[controller_index];
						if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
							ship->do_fire_1 = false;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
							ship->do_fire_3 = false;
						}
						break;
					case SDL_CONTROLLERAXISMOTION:
						controller_index = e.caxis.which;
						ship = ships[controller_index];
						if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
							if (abs(e.caxis.value) > DEAD_ZONE) {
								ship->move_dir_x = (int)e.caxis.value;
								ship->face_dir_x = (int)e.caxis.value;
							} else {
								ship->move_dir_x = 0;
							}
						} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
							if (abs(e.caxis.value) > DEAD_ZONE) {
								ship->move_dir_y = (int)e.caxis.value;
								ship->face_dir_y = (int)e.caxis.value;
							} else {
								ship->move_dir_y = 0;
							}
						} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
							double old_gun_mag = sqrt(pow(ship->gun_dir_x, 2) + pow(ship->gun_dir_y, 2));
							//if (controller_index == 1) std::cout << "x\t" << new_gun_mag << std::endl;
							int value = e.caxis.value / 16;
							//if (value > 2000) value = 2000;
							//if (value < -2000) value = -2000;
							double angle = ((double)value / 2000) * (3.14159 / 96);
							//std::cout << old_gun_mag << std::endl;
							ship->gun_dir_x = (int)(10000 * sin(angle));
							/*
							if (new_gun_mag >= SPACESHIP_MIN_GUN_DIR) {
								ship->gun_dir_x = (int)(e.caxis.value);
							} else {
								ship->gun_dir_x = (int)sqrt(pow(SPACESHIP_MIN_GUN_DIR, 2) - pow(ship->gun_dir_y, 2));
								if (e.caxis.value < 0) {
									//std::cout << new_gun_mag- SPACESHIP_MIN_GUN_DIR << std::endl;
									ship->gun_dir_x *= -1;
								}
							}
							*/
						} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
							double new_gun_mag = sqrt(pow(e.caxis.value, 2) + pow(ship->gun_dir_x, 2));
							//if (controller_index == 1) std::cout << "y\t" << new_gun_mag << std::endl;
							int value = e.caxis.value / 16;
							//if (value > 2000) value = 2000;
							//if (value < -2000) value = -2000;
							double angle = ((double)value / 2000) * (3.14159 / 96);
							//std::cout << angle << std::endl;
							ship->gun_dir_y = (int)(10000 * sin(angle));
							/*
							if (new_gun_mag >= SPACESHIP_MIN_GUN_DIR) {
								ship->gun_dir_y = (int)(e.caxis.value);
							} else {
								ship->gun_dir_y = (int)sqrt(pow(SPACESHIP_MIN_GUN_DIR,2) - pow(ship->gun_dir_x, 2));
								if (e.caxis.value < 0) {
									ship->gun_dir_y *= -1;
								}
							}
							*/
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
			for (int i = 0; i < num_players; i++) {
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

				// handle normal bullet spawns
				ship->fire_1();

				ship->fire_2();

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

					// update acceleration
					double accel_mag = 0.3*sqrt(pow(ship->move_dir_x, 2) + pow(ship->move_dir_y, 2));
					if (accel_mag > ship->max_accel) {
						accel_mag = ship->max_accel;
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
					if (ship->x_vel > 0) {
						ship->x_vel -= (int)((pow(ship->x_vel, 2) + ship->constant_friction) / ship->friction_limiter);
						if (ship->x_vel < 0) {
							ship->x_vel = 0;
						}
					} else if (ship->x_vel < 0) {
						ship->x_vel += (int)((pow(ship->x_vel, 2) + ship->constant_friction) / ship->friction_limiter);
						if (ship->x_vel > 0) {
							ship->x_vel = 0;
						}
					}

					if (ship->y_vel > 0) {
						ship->y_vel -= (int)((pow(ship->y_vel, 2) + ship->constant_friction) / ship->friction_limiter);
						if (ship->y_vel < 0) {
							ship->y_vel = 0;
						}
					} else if (ship->y_vel < 0) {
						ship->y_vel += (int)((pow(ship->y_vel, 2) + ship->constant_friction) / ship->friction_limiter);
						if (ship->y_vel > 0) {
							ship->y_vel = 0;
						}
					}

					// handle collisions between ships
					for (int j = 0; j < num_players; j++) {
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
						ship->x_pos = ship->respawn_x;
						ship->y_pos = ship->respawn_y;
						ship->x_vel = 0;
						ship->y_vel = 0;

						ship->percent = 0;
						ship->lives--;
						ship->invincibility_cooldown += ship->respawn_invincibility_delay;

						SDL_HapticRumblePlay(haptics[i], 1, 300);
					}

				}

				// update bullets
				ship->update_projectiles_1(STATUS_BAR_WIDTH * 10000, WINDOW_WIDTH * 10000, 0, WINDOW_HEIGHT * 10000, num_players, ships);

				// update missiles
				ship->update_projectiles_2(STATUS_BAR_WIDTH * 10000, WINDOW_WIDTH * 10000, 0, WINDOW_HEIGHT * 10000, num_players, ships);

				// Check to see if the game is over
				int victoryCheck = 0;
				for (int i = 0; i < num_players; i++) {
					ship = ships[i];
					if (ship->lives == 0) {
						victoryCheck++;
					}
				}
				if (victoryCheck == num_players - 1) {
					for (int j = 0; j < num_players; j++) {
						ship = ships[j];
						if (ship->lives != 0) {
							winner = j + 1;
						}
					}
					for (int k = 0; k < num_players; k++) {
						ship = ships[k];
						ship->lives = 4;
						ship->percent = 0;
						ship->stamina = ship->stamina_max;
					}
					currentState = results;
				}

			} // end of update ships

			// begin rendering
			{
				// render background
				render_texture(bg, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0, 1);
				render_texture(sun_tex, (WINDOW_WIDTH-STATUS_BAR_WIDTH) / 2 + STATUS_BAR_WIDTH, WINDOW_HEIGHT / 2, 0, 1);

				// render all ship elements
				for (int i = 0; i < num_players; i++) {

					Ship* ship = ships[i];

					ship->render();

					// render bullets
					ship->render_projectiles_1();

					// render missiles
					ship->render_projectiles_2();
					

				}

				// render UI elements
				{
					// render status bar background
					SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
					SDL_Rect s;
					s.x = 0;
					s.y = 0;
					s.w = STATUS_BAR_WIDTH;
					s.h = 1000;
					SDL_RenderFillRect(renderer, &s);

					int textAdjustment = 50; // todo: rename this shit

					// render each ship's UI elements
					for (int i = 0; i < num_players; i++) {
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
								SDL_SetRenderDrawColor(renderer, 0, 160, 0, SDL_ALPHA_OPAQUE);
							} else {
								SDL_SetRenderDrawColor(renderer, 210, 210, 0, SDL_ALPHA_OPAQUE);
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
							SDL_Color White = { 255, 255, 255 };
							SDL_Surface* percentSurface = TTF_RenderText_Blended(caladea36, str, White); //Create the sdl surface
							SDL_Texture* percentTexture = SDL_CreateTextureFromSurface(renderer, percentSurface); //Convert to texture
							SDL_Rect percentRect; //create a rect
							percentRect.x = 0; //controls the rect's x coordinate 
							percentRect.y = (60 + 100 * i + 1) + textAdjustment * i; // controls the rect's y coordinte
							SDL_QueryTexture(percentTexture, NULL, NULL, &percentRect.w, &percentRect.h);
							SDL_RenderCopy(renderer, percentTexture, NULL, &percentRect);
							SDL_DestroyTexture(percentTexture);
							SDL_FreeSurface(percentSurface);
						}

						// render stock counter
						{
							char playerLives[20];
							sprintf_s(playerLives, "Lives: %d", ship->lives);
							SDL_Color White = { 255, 255, 255 };
							SDL_Surface* stockSurface = TTF_RenderText_Blended(caladea36, playerLives, White); //Create the sdl surface
							SDL_Texture* stockTexture = SDL_CreateTextureFromSurface(renderer, stockSurface); //Convert to texture
							SDL_Rect stockRect; //create a rect
							stockRect.x = 0; //2controls the rect's x coordinate 
							stockRect.y = (10 + 150 * i); // controls the rect's y coordinte
							SDL_QueryTexture(stockTexture, NULL, NULL, &stockRect.w, &stockRect.h);
							SDL_RenderCopy(renderer, stockTexture, NULL, &stockRect);
							SDL_DestroyTexture(stockTexture);
							SDL_FreeSurface(stockSurface);
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
			{
				SDL_Color White = { 255, 255, 255 };  // Renders the color of the text
				SDL_Surface* pauseSurface = TTF_RenderText_Blended(caladea36, "Paused", White); //Create the sdl surface
				SDL_Texture* pauseTexture = SDL_CreateTextureFromSurface(renderer, pauseSurface); //Convert to texture
				SDL_Rect pauseRect; //create a rect
				pauseRect.x = WINDOW_WIDTH / 2;
				pauseRect.y = WINDOW_HEIGHT / 2;
				SDL_QueryTexture(pauseTexture, NULL, NULL, &pauseRect.w, &pauseRect.h);
				SDL_RenderCopy(renderer, pauseTexture, NULL, &pauseRect);
				SDL_DestroyTexture(pauseTexture);
				SDL_FreeSurface(pauseSurface);
			}

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
				SDL_Color White = { 255, 255, 255 };  // Renders the color of the text
				SDL_Surface* winnerSurface = TTF_RenderText_Blended(caladea36, winnerMessage, White); //Create the sdl surface
				SDL_Texture* winnerTexture = SDL_CreateTextureFromSurface(renderer, winnerSurface); //Convert to texture
				SDL_Rect winnerRect; //create a rect
				winnerRect.x = WINDOW_WIDTH / 3.5;
				winnerRect.y = WINDOW_HEIGHT / 2.5;
				SDL_QueryTexture(winnerTexture, NULL, NULL, &winnerRect.w, &winnerRect.h);
				SDL_RenderCopy(renderer, winnerTexture, NULL, &winnerRect);
				SDL_DestroyTexture(winnerTexture);
				SDL_FreeSurface(winnerSurface);

				SDL_Surface* promptSurface = TTF_RenderText_Blended(caladea36, "Press the A button to continue.", White); //Create the sdl surface
				SDL_Texture* promptTexture = SDL_CreateTextureFromSurface(renderer, promptSurface); //Convert to texture
				SDL_Rect promptRect; //create a rect
				promptRect.x = WINDOW_WIDTH / 3.5;
				promptRect.y = WINDOW_HEIGHT / 2;
				SDL_QueryTexture(promptTexture, NULL, NULL, &promptRect.w, &promptRect.h);
				SDL_RenderCopy(renderer, promptTexture, NULL, &promptRect);
				SDL_DestroyTexture(promptTexture);
				SDL_FreeSurface(promptSurface);
			}
			SDL_RenderPresent(renderer);
		}

		
	} //end of loop

	for (int i = 0; i < num_players; i++) {
		// todo: close the controllers themselves?
		SDL_HapticClose(haptics[i]);
	}
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
		if (e->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
			quit = true;
			event_eaten = true;
		}
		break;
	}
	return event_eaten;
}

void render_character_selector(int x, int y, SDL_Texture* ship_tex, SDL_Texture* right_arrow, SDL_Texture* left_arrow) {
	render_texture(ship_tex, x+300, y+150, 0, 4);
	render_texture(right_arrow, x+400, y+150, 0, 1);
	render_texture(left_arrow, x+200, y+150, 0, 1);
}
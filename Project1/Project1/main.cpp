#include <iostream>
#include <string>
#include <math.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <time.h>

#include "spaceship.h"
#include "grizzly.h"
#include "black.h"
#include "polar.h"

#include "item.h"

#include "asteroid.h"

#include "renderer.h"

SDL_Renderer* renderer;

SDL_Window* window;

struct lra {
	bool l = false;
	bool r = false;
	bool a = false;
};

const int WIDTH_UNITS = 10000 * 1280;
const int HEIGHT_UNITS = 10000 * 720;
const int BARSIZE = 10000 * 26;
const int STATUS_BAR_WIDTH = 10000 * 150;

bool dpad_down[4] = { false, false, false, false };

Renderer* r;

bool quit = false;
bool is_fullscreen = false;
bool xp_mode = false;
bool muted = true;

int controller_mappings[4] = { -1, -1, -1, -1 };
SDL_GameController* controllers[4] = { NULL, NULL, NULL, NULL };
SDL_Haptic* haptics[4];
lra LRA[4];

Mix_Music* music;

bool read_global_input(SDL_Event* e);

enum ship_type {
	black = 0,
	grizzly = 1,
	polar = 2,
};

enum wrap_type {
	none,
	direct,
	inverse,
};

enum stage {
	anchorage, // normal
	fairbanks, // asteroid in middle
	juneau, // random asteroids
};

stage selected_stage = juneau;

void render_plugin_to_join(int x, int y);
int lookup_controller(int instanceID);
void render_character_selector(int x, int y, SDL_Texture* ship_tex, ship_type shipType, SDL_Texture* right_arrow, SDL_Texture* left_arrow, bool ready);
void render_results(int x, int y, SDL_Texture * ship_tex, Ship * ship);

int main(int, char**) {

	enum gameState {
		mainMenu,
		options,
		characterSelect,
		stageSelect,
		initGame,
		inGame,
		results,
		pause,
	};

	enum color {
		red = 0,
		blue = 1,
		yellow = 2,
		green = 3,
	};

	gameState currentState = mainMenu;

	// init SDL
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0) {
		std::cout << "SDL_Init error: " << SDL_GetError() << std::endl;
		return 1;
	}

	// init sound
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
	}

	// init random numbers
	srand(time(NULL));
	

	int WINDOW_WIDTH;
	int WINDOW_HEIGHT;

	// set resolution
	enum Resolution {
		_480p,
		_720p,
		_1080p,
		_1440p,
		_2160p // 4k
	};
	const Resolution res = _1080p;
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

	window = SDL_CreateWindow("Alaskan Cosmobear Spacefighting", 675, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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

	const int game_start_delay = 4 * 60;
	int game_start_cooldown;

	const double CONTROLLER_MAX_ANGLE = M_PI / 6;
	const int DEAD_ZONE = 5000;

	const int playerUiWidth = 10;

	r = new Renderer(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

	// load textures
	SDL_Texture* bg = r->LoadTexture("..\\Project1\\assets\\background.png");

	SDL_Texture* ship_textures[3][4];
	ship_textures[black][red] = r->LoadTexture("..\\Project1\\assets\\ships\\black-red.png");
	ship_textures[black][blue] = r->LoadTexture("..\\Project1\\assets\\ships\\black-blue.png");
	ship_textures[black][yellow] = r->LoadTexture("..\\Project1\\assets\\ships\\black-yellow.png");
	ship_textures[black][green] = r->LoadTexture("..\\Project1\\assets\\ships\\black-green.png");

	ship_textures[grizzly][red] = r->LoadTexture("..\\Project1\\assets\\ships\\grizzly-red.png");
	ship_textures[grizzly][blue] = r->LoadTexture("..\\Project1\\assets\\ships\\grizzly-blue.png");
	ship_textures[grizzly][yellow] = r->LoadTexture("..\\Project1\\assets\\ships\\grizzly-yellow.png");
	ship_textures[grizzly][green] = r->LoadTexture("..\\Project1\\assets\\ships\\grizzly-green.png");

	ship_textures[polar][red] = r->LoadTexture("..\\Project1\\assets\\ships\\polar-red.png");
	ship_textures[polar][blue] = r->LoadTexture("..\\Project1\\assets\\ships\\polar-blue.png");
	ship_textures[polar][yellow] = r->LoadTexture("..\\Project1\\assets\\ships\\polar-yellow.png");
	ship_textures[polar][green] = r->LoadTexture("..\\Project1\\assets\\ships\\polar-green.png");
	
	SDL_Texture* right_arrow = r->LoadTexture("..\\Project1\\assets\\right_arrow.png");
	SDL_Texture* left_arrow = r->LoadTexture("..\\Project1\\assets\\left_arrow.png");

	

	Mix_Chunk* beep = Mix_LoadWAV("..\\Project1\\assets\\sounds\\confirm.wav");
	Mix_Chunk* selected_ship = Mix_LoadWAV("..\\Project1\\assets\\sounds\\beep.wav");
	Mix_Chunk* powerup_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\item.wav");
	Mix_Chunk* death_sfx = Mix_LoadWAV("..\\Project1\\assets\\sounds\\death.wav");


	//music = Mix_LoadMUS("..\\Project1\\assets\\sounds\\Cyborg_Ninja.wav");
	// "Cyborg Ninja" Kevin MacLeod (incompetech.com)
	// Licensed under Creative Commons : By Attribution 3.0 License
	// http ://creativecommons.org/licenses/by/3.0/
	if (!muted) {
		Mix_PlayMusic(music, -1);
		Mix_AllocateChannels(32);
	} else {
		Mix_AllocateChannels(0);
	}

	Mix_VolumeMusic(MIX_MAX_VOLUME / 8);

	// stage stuff
	int asteroid_spawn_cooldown = 1;

	ship_type selections[4] = { grizzly, grizzly, grizzly, grizzly };
	bool analog_stick_moved[4] = { false, false, false, false };
	bool ready[4] = { false, false, false, false };
	//bool ready[4] = { true, true, true, true };


	bool do_items = false;

	SDL_Event e;
	
	int winner;

	Ship* ships[4] = { NULL, NULL, NULL, NULL };

	Item* items[100];
	int num_items = 0;

	int item_spawn_cooldown = 60 * 30;
	
	Asteroid* asteroids[100];
	int num_asteroids = 0;


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
			r->render_solid_bg();

			// render title name and prompt to move forward
			{
				r->render_text(WIDTH_UNITS / 2,  2 * HEIGHT_UNITS / 5, "Alaskan Cosmobear Spacefighting", true, false, false, large_f, 255, 255);

				r->render_text(WIDTH_UNITS / 2, HEIGHT_UNITS / 2, "Press the A button to start.", true, false, false, medium_f, 255, 255);

				r->render_text(WIDTH_UNITS / 2, 10 * HEIGHT_UNITS / 17, "Press the B button to quit.", true, false, false, medium_f, 255, 255);
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
						bool all_ready = true;
						for (int i = 0; i < 4; i++) {
							if (controllers[i] && !ready[i]) {
								all_ready = false;
							}
						}
						if (all_ready) {
							
							currentState = stageSelect;
						} else {

							if (!ready[controller_index]) {
								Mix_PlayChannel(-1, selected_ship, 0);
							}
							ready[controller_index] = true;
						}
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
						bool all_ready = true;
						for (int i = 0; i < 4; i++) {
							if (controllers[i] && !ready[i]) {
								all_ready = false;
							}
						}
						if (all_ready) {
							currentState = stageSelect;
						}
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
						if (ready[controller_index] == false) {
							currentState = mainMenu;
						} else {
							ready[controller_index] = false;
						}
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_Y) {
						do_items = !do_items;
						Mix_PlayChannel(-1, beep, 0);

					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
						if (ready[controller_index]) break;
						Mix_PlayChannel(-1, beep, 0);
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
						if (ready[controller_index]) break;
						Mix_PlayChannel(-1, beep, 0);
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
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
						if (ready[controller_index]) break;
						Mix_PlayChannel(-1, beep, 0);
						int random_selection = rand() % 3;
						selections[controller_index] = (ship_type)random_selection;
					}
					break;
				case SDL_CONTROLLERAXISMOTION:
					controller_index = lookup_controller(e.caxis.which);
					if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
						int min_angle = 26000;

						if (e.caxis.value > min_angle && !analog_stick_moved[controller_index]) {
							if (ready[controller_index]) break;
							analog_stick_moved[controller_index] = true;
							Mix_PlayChannel(-1, beep, 0);
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
						} else if (e.caxis.value < -min_angle && !analog_stick_moved[controller_index]) {
							if (ready[controller_index]) break;
							analog_stick_moved[controller_index] = true;
							Mix_PlayChannel(-1, beep, 0);
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
						} else if (e.caxis.value < min_angle && e.caxis.value > -min_angle) {
							analog_stick_moved[controller_index] = false;
						}
					}
					break;
				}
			}

			// render basic black background
			r->render_solid_bg();

			// render item toggle
			if (do_items) {
				r->render_text(0, 0, " Items are ON ", false, false, true, medium_f, 255, 255);
			} else {
				r->render_text(0, 0, " Items are OFF ", false, false, true, medium_f, 255, 255);
			}

			r->render_cross_bars(BARSIZE);
			
			// render ships selections
			{
				if (controllers[0]) {
					render_character_selector(0, 0, ship_textures[selections[0]][red], selections[0], right_arrow, left_arrow, ready[0]);
				} else {
					render_plugin_to_join(0, 0);
				}
				if (controllers[1]) {
					render_character_selector(WIDTH_UNITS / 2 + BARSIZE/2, 0, ship_textures[selections[1]][blue], selections[1], right_arrow, left_arrow, ready[1]);
				} else {
					render_plugin_to_join(WIDTH_UNITS / 2 + BARSIZE/2, 0);
				}
				if (controllers[2]) {
					render_character_selector(0, HEIGHT_UNITS / 2 + BARSIZE/2, ship_textures[selections[2]][yellow], selections[2], right_arrow, left_arrow, ready[2]);
				} else {
					render_plugin_to_join(0, HEIGHT_UNITS / 2 + BARSIZE/2);
				}
				if (controllers[3]) {
					render_character_selector(WIDTH_UNITS / 2 + BARSIZE/2, HEIGHT_UNITS / 2 + BARSIZE/2, ship_textures[selections[3]][green], selections[3], right_arrow, left_arrow, ready[3]);
				} else {
					render_plugin_to_join(WIDTH_UNITS / 2 + BARSIZE/2, HEIGHT_UNITS / 2 + BARSIZE/2);
				}
			}

			// render "press start to begin"
			bool all_ready = true;
			for (int i = 0; i < 4; i++) {
				if (controllers[i] && !ready[i]) {
					all_ready = false;
				}
			}
			if (all_ready) {
				r->render_text(WIDTH_UNITS / 2, HEIGHT_UNITS / 2, " Press Start to begin! ", true, true, true, large_f, 255, 255);
			}

			SDL_RenderPresent(renderer);

		}
		// start of stage select state
		else if (currentState == stageSelect) {
			currentState = initGame;
		}
		else if (currentState == initGame) {
			// begin start game timer
			game_start_cooldown = game_start_delay;

			// reset 'ready' status of all players
			for (int i = 0; i < 4; i++) {
				ready[i] = false;
			}

			// init game objects
			// init asteroids
			if (selected_stage == fairbanks) {
				asteroids[num_asteroids] = new Asteroid((WIDTH_UNITS - STATUS_BAR_WIDTH) / 2 + STATUS_BAR_WIDTH, HEIGHT_UNITS / 2, 0, 0, r);
				num_asteroids = 1;
			}

			// init ships
			const int spawn_locations_x[4] = {
				(WIDTH_UNITS - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH ,
				3 * (WIDTH_UNITS - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH,
				2 * (WIDTH_UNITS - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH,
				2 * (WIDTH_UNITS - STATUS_BAR_WIDTH) / 4 + STATUS_BAR_WIDTH
			};
			const int spawn_locations_y[4] = {
				HEIGHT_UNITS / 2,
				HEIGHT_UNITS / 2,
				HEIGHT_UNITS / 4,
				3 * HEIGHT_UNITS / 4
			};
			// todo: remove this
			int num_ships = 0;
			for (int i = 0; i < 4; i++) {
				if (ships[i]) {
					delete ships[i];
					ships[i] = NULL;
				}

				if (!controllers[i]) continue;

				num_ships++;

				switch (selections[i]) {
				case black:
					ships[i] = new Black(i, spawn_locations_x[i], spawn_locations_y[i], r);
					break;
				case grizzly:
					ships[i] = new Grizzly(i, spawn_locations_x[i], spawn_locations_y[i], r);
					break;
				case polar:
					ships[i] = new Polar(i, spawn_locations_x[i], spawn_locations_y[i], r);
					break;
				}
			}

			int num_lives = 0;
			if (num_ships == 2) num_lives = 4;
			if (num_ships == 3) num_lives = 5;
			if (num_ships == 4)  num_lives = 5;

			for (int i = 0; i < 4; i++) {
				if (!controllers[i]) continue;

				ships[i]->lives = num_lives;
			}

			// render background once; this is needed for xp mode
			r->render_texture(bg, WIDTH_UNITS / 2, HEIGHT_UNITS / 2, 0, 1);

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
						if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
							ship->do_fire_1 = true;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
							ship->do_fire_3 = true;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSTICK) {
							ship->do_speed_boost = true;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
							if (game_start_cooldown < 2 * 60) {
								currentState = pause;
							}
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
							//std::cout << circle_x << std::endl;
							ship->right_stick_x = (int)(10000 * circle_x);
							double right_stick_mag = sqrt(pow(ship->right_stick_x, 2) + pow(ship->right_stick_y, 2));
							//std::cout << right_stick_mag << std::endl;
							if (right_stick_mag > DEAD_ZONE) {
								ship->desired_gun_dir_x = ship->right_stick_x;
								ship->desired_gun_dir_y = ship->right_stick_y;
							}

						} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
							double value = (double)e.caxis.value / 32768;

							double circle_y = sin(value * CONTROLLER_MAX_ANGLE) / sin(CONTROLLER_MAX_ANGLE);
							ship->right_stick_y = (int)(10000 * circle_y);
							double right_stick_mag = sqrt(pow(ship->right_stick_x, 2) + pow(ship->right_stick_y, 2));
							//std::cout << right_stick_mag << std::endl;
							if (right_stick_mag > DEAD_ZONE) {
								ship->desired_gun_dir_x = ship->right_stick_x;
								ship->desired_gun_dir_y = ship->right_stick_y;
							}

						} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
							int min_activation = 20000;
							int min_deactivation = 18000;
							if (e.caxis.value < min_deactivation) {
								ship->do_fire_2 = false;
							} else if (e.caxis.value > min_activation) {
								ship->do_fire_2 = true;
							}
						} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
							int min_activation = 20000;
							int min_deactivation = 18000;
							if (e.caxis.value < min_deactivation) {
								ship->do_fire_2 = false;
							} else if (e.caxis.value > min_activation) {
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
			// spawn asteroids
			if (selected_stage == juneau) {
				asteroid_spawn_cooldown--;
				if (asteroid_spawn_cooldown == 0) {
					asteroid_spawn_cooldown = 20 * 60;

					asteroids[num_asteroids] = new Asteroid(WIDTH_UNITS / 2, 0, 0, 40000, r);
					num_asteroids++;
				}
			}

			// spawn items
			if (do_items) {
				item_spawn_cooldown--;
				if (item_spawn_cooldown == 0) {
					item_spawn_cooldown = (20 * 60) + (rand() % (10 * 60));
					int x_pos = 10000 * (rand() % ((WIDTH_UNITS - STATUS_BAR_WIDTH) / 10000)) + STATUS_BAR_WIDTH;
					int y_pos = 10000 * (rand() % (HEIGHT_UNITS / 10000));
					int type = rand() % NUM_ITEM_TYPES;
					items[num_items] = new Item(x_pos, y_pos, (item_type)type, r);
					num_items++;
				}
			}

			// update asteroids
			for (int i = 0; i < num_asteroids; i++) {
				Asteroid* a = asteroids[i];
				a->update();
			}

			// update ships
			for (int i = 0; i < 4; i++) {
				if (!ships[i]) continue;
				Ship* ship = ships[i];

				if (ship->lives == 0) continue;

				ship->update();
				if (game_start_cooldown > 1 * 60) continue;

				// reduce invincibility time
				if (ship->invincibility_cooldown > 0) {
					ship->invincibility_cooldown--;
				}


				// regen stamina
				ship->stamina += ship->stamina_per_frame;
				if (ship->stamina > ship->stamina_max) {
					ship->stamina = ship->stamina_max;
				}

				// handle projectile spawns
				ship->fire_1();

				ship->fire_2();

				ship->fire_3();

				// update ship itself
				{

					// item timers
					for (int j = 0; j < NUM_ITEM_TYPES; j++) {
						if (ships[i]->item_times[j] > 0) {
							ships[i]->item_times[j]--;
						}
					}

					if (ship->item_times[small] > 0) {
						ship->radius = ship->normal_radius / 2;
						ship->gun_length = ship->normal_gun_length / 2;
					} else {
						ship->radius = ship->normal_radius;
						ship->gun_length = ship->normal_gun_length;
					}

					if (ship->speed_boost_cooldown > 0) {
						ship->speed_boost_cooldown--;
					}

					if (ship->invincibility_cooldown < 2 * 60) {

						// update acceleration
						double accel_mag = sqrt(pow(ship->move_dir_x, 2) + pow(ship->move_dir_y, 2));
						if (accel_mag > ship->max_accel) {
							accel_mag = ship->max_accel;
						}
						if (ship->do_speed_boost && ship->speed_boost_cooldown == 0) {
							accel_mag = ship->max_accel * 40;
							ship->speed_boost_cooldown += ship->speed_boost_delay;
							ship->do_speed_boost = false;
						}
						if (ship->item_times[speed_up] > 0) {
							accel_mag *= 2;
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
							// todo: scale linearly with velocity?
							double friction_accel = ((pow(total_vel, 2) + ship->constant_friction) / ship->friction_limiter) / iterations_per_frame;

							int friction_accel_x = (int)(((double)friction_accel * ship->x_vel) / total_vel);
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

							int friction_accel_y = (int)(((double)friction_accel * ship->y_vel) / total_vel);
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
							if (ships[j]->lives == 0) continue;
							double dist = sqrt(pow(ship->x_pos - ships[j]->x_pos, 2) + pow(ship->y_pos - ships[j]->y_pos, 2));
							if (dist == 0) dist = 1;
							if (dist <= (ships[i]->radius + ships[j]->radius)) {
								double total_force = 140000000000000000.0 / pow(dist, 2);
								double x_force = (ship->x_pos - ships[j]->x_pos) * total_force / dist;
								ship->x_vel += (int)(x_force / ship->weight);
								double y_force = (ship->y_pos - ships[j]->y_pos) * total_force / dist;
								ship->y_vel += (int)(y_force / ship->weight);
							}
						}
					}

					// check for collisions with items
					for (int j = 0; j < num_items; j++) {
						Item* item = items[j];

						if (item->picked_up) {
							item->y_pos -= 6000;
							item->alpha -= 2;

							if (item->alpha <= 0) {
								num_items--;
								delete item;
								items[j] = items[num_items];
							}
						} else {
							double dist = sqrt(pow(ship->x_pos - item->x_pos, 2) + pow(ship->y_pos - item->y_pos, 2));
							if (dist < (ship->radius + item->radius)) {
								ship->item_times[item->type] += 600;
								item->picked_up = true;
								Mix_PlayChannel(-1, powerup_sfx, 0);
							}
						}
					}

					// check for collisions with asteroids
					for (int j = 0; j < num_asteroids; j++) {
						Asteroid* a = asteroids[j];
						double dist = sqrt(pow(ship->x_pos - a->x_pos, 2) + pow(ship->y_pos - a->y_pos, 2));
						if (dist < (ship->radius + a->radius)) {

							/*
							double total_force = 20000000000000000000.0 / pow(dist, 2);
							double x_force = (ship->x_pos - a->x_pos) * total_force / dist;
							ship->x_vel += (int)(x_force / ship->weight);
							double y_force = (ship->y_pos - a->y_pos) * total_force / dist;
							ship->y_vel += (int)(y_force / ship->weight);

							ship->percent += 28;
							*/

							ship->take_knockback(ship->x_pos - a->x_pos, ship->y_pos - a->y_pos, 100, 20, 28, haptics[j]);
						}
					}

					// update position
					ship->x_pos += ship->x_vel;
					ship->y_pos += ship->y_vel;

					// handle death
					if (ship->x_pos < (STATUS_BAR_WIDTH - ship->radius) || ship->x_pos > (WIDTH_UNITS + ship->radius) 
							|| ship->y_pos < (0 - ship->radius) || ship->y_pos > (HEIGHT_UNITS + ship->radius)) {
						if (ship->item_times[bounce] > 0 || ship->invincibility_cooldown > 0) {
							if (ship->x_pos < STATUS_BAR_WIDTH && ship->x_vel < 0) {
								ship->x_vel *= -1;
							} else if (ship->x_pos > WIDTH_UNITS && ship->x_vel > 0) {
								ship->x_vel *= -1;
							}
							else if (ship->y_pos < 0 && ship->y_vel < 0) {
								ship->y_vel *= -1;
							} else if (ship->y_pos > HEIGHT_UNITS && ship->y_vel > 0) {
								ship->y_vel *= -1;
							}

						} else {

							ship->lives--;

							int last_hit = ship->last_hit;
							if (last_hit != -1) {
								ships[last_hit]->kills[ships[last_hit]->num_kills] = i;
								ships[last_hit]->num_kills++;
								if (ship->id == last_hit) {
									ship->num_suicides++;
								}
							} else {
								// todo: fix this
								//ship->num_suicides++;
							}


							SDL_HapticRumblePlay(haptics[i], 1, 300);

							if (ship->lives <= 0) {
								ship->die();
								ship->lives = 0;
							}

							ship->invincibility_cooldown += ship->respawn_invincibility_delay;

							ship->x_pos = ship->respawn_x;
							ship->y_pos = ship->respawn_y;
							ship->x_vel = 0;
							ship->y_vel = 0;

							ship->percent = 0;

							ship->stamina = ship->stamina_max;

							ship->last_hit = -1;

							Mix_PlayChannel(-1, death_sfx, 0);

						}
					}

				}

				// update projectiles
				ship->update_projectiles_1(STATUS_BAR_WIDTH, WIDTH_UNITS, 0, HEIGHT_UNITS, ships, asteroids, num_asteroids, haptics);
				ship->update_projectiles_2(STATUS_BAR_WIDTH, WIDTH_UNITS, 0, HEIGHT_UNITS, ships, asteroids, num_asteroids, haptics);
				ship->update_projectiles_3(STATUS_BAR_WIDTH, WIDTH_UNITS, 0, HEIGHT_UNITS, ships, asteroids, num_asteroids, haptics);

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

					// remove items
					for (int j = 0; j < num_items; j++) {
						delete items[j];
					}
					num_items = 0;

					// halt all sound effects
					Mix_HaltChannel(-1);
				}

			} // end of update ships

			if (currentState == results) {
				continue;
			}

			// begin rendering
			{
				// render background
				if (!xp_mode) {
					r->render_texture(bg, WIDTH_UNITS / 2, HEIGHT_UNITS / 2, 0, 1.02);
				}

				// render game start countdown
				if (game_start_cooldown > 0) {
					game_start_cooldown--;
					std::string s;
					if (game_start_cooldown > 3 * 60) {
						s = " 3 ";
					} else if (game_start_cooldown > 2 * 60) {
						s = " 2 ";
					} else if (game_start_cooldown > 1 * 60) {
						s = " 1 ";
					} else {
						s = " GO! ";
					}
					r->render_text((WIDTH_UNITS-STATUS_BAR_WIDTH) / 2 + STATUS_BAR_WIDTH, HEIGHT_UNITS / 2, s, true, true, true, large_f, 255, 255);

				}

				// render all ship elements
				for (int i = 0; i < 4; i++) {
					if (!ships[i]) continue;
					Ship* ship = ships[i];
					if (ship->lives == 0) continue;
					ship->render();
				}

				for (int i = 0; i < 4; i++) {
					if (!ships[i]) continue;
					Ship* ship = ships[i];
					if (ship->lives == 0) continue;

					// render projectiles
					ship->render_projectiles_1();
					ship->render_projectiles_2();
					ship->render_projectiles_3();
				}

				// render items
				for (int i = 0; i < num_items; i++) {
					items[i]->render();
				}

				// render asteroids
				for (int i = 0; i < num_asteroids; i++) {
					asteroids[i]->render();
				}

				// render UI elements
				{
					// render status bar background
					r->SetRenderDrawColor(128, 128, 128, SDL_ALPHA_OPAQUE);
					r->render_rect(0, 0, STATUS_BAR_WIDTH, HEIGHT_UNITS);

					int box_height = 150;

					// render each ship's UI elements
					for (int i = 0; i < 4; i++) {
						if (!ships[i]) continue;
						Ship* ship = ships[i];
						if (ship->lives == 0) continue;
						
						// render stamina bar
						{
							// set stamina bar color
							if (ship->id == 0) {
								r->SetRenderDrawColor(160, 0, 0, SDL_ALPHA_OPAQUE);
							} else if (ship->id == 1) {
								r->SetRenderDrawColor(0, 0, 160, SDL_ALPHA_OPAQUE);
							} else if (ship->id == 2) {
								r->SetRenderDrawColor(210, 210, 0, SDL_ALPHA_OPAQUE);
							} else {
								r->SetRenderDrawColor(0, 160, 0, SDL_ALPHA_OPAQUE);
							}

							r->render_rect(0, 10000 * (120 + box_height*i), (int)(((double)STATUS_BAR_WIDTH * ship->stamina) / ship->stamina_max), 10000 * 30);
						}

						// render percentages
						{
							char str[10];
							snprintf(str, 10, "P%d: %d%%", i + 1, ship->percent);
							int blue_and_green = 255 - (int)(255 * ship->percent / 300);
							int alpha = 255;
							if (blue_and_green < 0) {
								blue_and_green = 0;
								alpha = 255 - (int)(255 * (ship->percent - 300) / 2000);
							}
							r->render_text(0, 10000 * (61 + box_height * i), str, false, false, false, medium_f, blue_and_green, alpha);
						}

						// render stock counter
						{
							char playerLives[20];
							sprintf_s(playerLives, "Lives: %d", ship->lives);
							r->render_text(0, 10000 * (10 + box_height * i), playerLives, false, false, false, medium_f, 255, 255);
						}
					}
				}
				SDL_RenderPresent(renderer);
			}
		} //end of ingame state

		// start pause state
		else if (currentState == pause) {
			while (SDL_PollEvent(&e)) {
				int controller_index;
				if (read_global_input(&e)) continue;
				switch (e.type) {
				case SDL_CONTROLLERBUTTONDOWN:
					controller_index = lookup_controller(e.cbutton.which);
					if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
						if (LRA[controller_index].a == true && LRA[controller_index].l == true && LRA[controller_index].r == true) {
							// reset buttons
							for (int i = 0; i < 4; i++) {
								LRA[i].a = false;
								LRA[i].l = false;
								LRA[i].r = false;
							}
							winner = -1;
							currentState = results;
						}	
						else {
							// reset buttons
							for (int i = 0; i < 4; i++) {
								LRA[i].a = false;
								LRA[i].l = false;
								LRA[i].r = false;
							}
							currentState = inGame;
						}
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
						LRA[controller_index].a = true;
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
						LRA[controller_index].l = true;
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
						LRA[controller_index].r = true;
					}
					break;
				case SDL_CONTROLLERBUTTONUP:
					controller_index = lookup_controller(e.cbutton.which);
					if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
						LRA[controller_index].a = false;
					} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
						LRA[controller_index].l = false;
					} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
						LRA[controller_index].r = false;
					}
					break;
				case SDL_CONTROLLERAXISMOTION:
					controller_index = lookup_controller(e.caxis.which);
					if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
						int min_activation = 20000;
						//std::cout << e.caxis.value << std::endl;
						if (e.caxis.value >= min_activation) {
							LRA[controller_index].r = true;
						} else {
							LRA[controller_index].r = false;
						}
					}
					else if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
						int min_activation = 20000;
						//std::cout << e.caxis.value << std::endl;
						if (e.caxis.value >= min_activation) {
							LRA[controller_index].l = true;
						} else {
							LRA[controller_index].l = false;
						}
					}
					break;
				}
			}

			// render word "paused"
			SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
			r->render_text((WIDTH_UNITS-STATUS_BAR_WIDTH) / 2 + STATUS_BAR_WIDTH, HEIGHT_UNITS / 2, " Paused ", true, true, true, medium_f, 255, 255);


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
						currentState = characterSelect;
					}
					break;
				}
			}

			// render basic black background
			r->render_solid_bg();

			// render who won plus continue prompt
			{
				char winnerMessage[15];

				if (winner == -1) {
					sprintf_s(winnerMessage, "No Contest!");
				}
				else {
					sprintf_s(winnerMessage, "Player %d wins!", winner);
				}
				r->render_text(WIDTH_UNITS / 2, HEIGHT_UNITS / 12, winnerMessage, true, false, false, medium_f, 255, 255);

				int start_height = HEIGHT_UNITS / 4;
				if (ships[0] != NULL) {
					render_results(WIDTH_UNITS / 8, start_height, ship_textures[selections[0]][red], ships[0]);
				} 
				if (ships[1] != NULL) {
					render_results(3 * WIDTH_UNITS / 8, start_height, ship_textures[selections[1]][blue], ships[1]);
				}
				if (ships[2] != NULL) {
					render_results(5 * WIDTH_UNITS / 8, start_height, ship_textures[selections[2]][yellow], ships[2]);
				}
				if (ships[3] != NULL) {
				render_results(7 * WIDTH_UNITS / 8, start_height, ship_textures[selections[3]][green], ships[3]);
				}
				r->render_text(WIDTH_UNITS / 2, 4 * HEIGHT_UNITS / 5, "Press the A button to continue.", true, false, false, medium_f, 255, 255);
			}
			SDL_RenderPresent(renderer);
		}

		
	} //end of loop

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}


bool read_global_input(SDL_Event* e) {
	bool event_eaten = false;
	
	switch (e->type) {
	case SDL_QUIT:
		quit = true;
		event_eaten = true;
		break;
	case SDL_KEYDOWN:
	{
		SDL_Keycode k = e->key.keysym.sym;
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
		} else if (k == SDLK_m) {
			muted = !muted;
			if (muted) {
				Mix_HaltMusic();
				Mix_AllocateChannels(0);
			} else {
				Mix_PlayMusic(music, -1);
				// the last 8 channels are reserved for continuous sfx
				Mix_AllocateChannels(32);
			}
			
			event_eaten = true;
		} else if (k == SDLK_q) {
			quit = true;

			event_eaten = true;
		}
		break;
	}
	case SDL_CONTROLLERBUTTONDOWN:
		if (e->cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
			quit = true;
			event_eaten = true;
		} else if (e->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
			//dpad_down[]
			//xp_mode = !xp_mode;
			//event_eaten = true;
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
					std::cout << SDL_GetError() << std::endl;
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



void render_plugin_to_join(int x, int y) {
	int box_w = (WIDTH_UNITS - BARSIZE) / 2;
	int box_h = (HEIGHT_UNITS - BARSIZE) / 2;

	r->render_text(x + box_w / 2, y + box_h / 3, "Plug in controller to join", true, false, false, medium_f, 255, 255);
}


int lookup_controller(int instanceID) {
	for (int i = 0; i < 4; i++) {
		if (controller_mappings[i] == instanceID) {
			return i;
		}
	}
	return -1;
}

void render_character_selector(int x, int y, SDL_Texture* ship_tex, ship_type shipType, SDL_Texture* right_arrow, SDL_Texture* left_arrow, bool ready) {
	int box_w = (WIDTH_UNITS - BARSIZE) / 2;
	int box_h = (HEIGHT_UNITS - BARSIZE) / 2;

	if (!ready) {
		r->render_texture(ship_tex, x + box_w / 2, y + box_h / 5, 0, 4);
		r->render_texture(left_arrow, x + box_w / 4, y + box_h / 5, 0, 1);
		r->render_texture(right_arrow, x + 3 * box_w / 4, y + box_h / 5, 0, 1);
	} else {
		r->render_texture(ship_tex, x + box_w / 2, y + box_h / 5, 0, 4.9);
	}

	std::string name;
	std::string wep1;
	std::string wep2;
	std::string wep3;
	if (shipType == 0) {
		name = "BLACK";
		wep1 = "RB: Burst Shot";
		wep2 = "RT, LT: Charge Shot";
		wep3 = "LB: Flamethrower";
	} else if (shipType == 1) {
		name = "GRIZZLY";
		wep1 = "RB: Bullets";
		wep2 = "RT, LT: Missiles";
		wep3 = "LB: Mines";
	} else {
		name = "POLAR";
		wep1 = "RB: Shotgun";
		wep2 = "RT, LT: Gravity Missiles";
		wep3 = "LB: Laser";
	}
	r->render_text(x + box_w / 2, y + 2 * box_h / 5, name, true, false, false, medium_f, 255, 255);
	r->render_text(x + box_w / 3, y + 6 * box_h / 10, wep1, false, false, false, medium_f, 255, 255);
	r->render_text(x + box_w / 3, y + 7 * box_h / 10, wep2, false, false, false, medium_f, 255, 255);
	r->render_text(x + box_w / 3, y + 8 * box_h / 10, wep3, false, false, false, medium_f, 255, 255);
}

void render_results(int x, int y, SDL_Texture * ship_tex, Ship * ship) {

	char killResult[15];
	char suicideResult[15];
	char damageGiven[30];
	char damageTaken[30];
	sprintf_s(killResult, "Kills: %d", ship->num_kills - ship->num_suicides);
	sprintf_s(suicideResult, "Self-Kills: %d", ship->num_suicides);
	sprintf_s(damageGiven, "Damage Given: %d%% ", ship->damage_done);
	sprintf_s(damageTaken, "Damage Taken: %d%%", ship->damage_taken);

	r->render_texture(ship_tex, x, y, 1, 5);
	r->render_text(x, y + 10000 * 100, killResult, true, false, false, small_f, 255, 255);
	r->render_text(x, y + 10000 * 130, suicideResult, true, false, false, small_f, 255, 255);
	r->render_text(x, y + 10000 * 160, damageGiven, true, false, false, small_f, 255, 255);
	r->render_text(x, y + 10000 * 190, damageTaken, true, false, false, small_f, 255, 255);
}
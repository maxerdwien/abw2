#include <iostream>
#include <string>

#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "spaceship.h"
#include "bullet.h"
#include "missile.h"

void render_texture(SDL_Texture*, SDL_Renderer*, int x, int y, double angle, double scaling);
double calculate_angle(int x_vel, int y_vel);
bullet** spawn_bullets(spaceship* ship, int velocity, int spread, int damage, int base_knockback, int knockback_scaling);
missile** spawn_missiles(spaceship* ship, int velocity, int spread, int damage, int base_knockback, int knockback_scaling);

int main(int, char**) {
	int test = TTF_Init(); // todo: delete or rename

	enum gameState {
		mainMenu,
		options,
		characterSelect,
		stageSelect,
		inGame,
		results,
		pause
	};

	//gameState currentState = mainMenu;
	gameState currentState = inGame;

	// init SDL
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0) {
		std::cout << "SDL_Init error: " << SDL_GetError() << std::endl;
		return 1;
	}

	// set resolution
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


	const int DEAD_ZONE = 5000;
	const int STATUS_BAR_WIDTH = 150;
	const int playerUiWidth = 10;

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
	SDL_Texture* bg = IMG_LoadTexture(renderer, "..\\Project1\\assets\\background.png");
	if (!bg) {
		std::cout << "error loading background: " << SDL_GetError() << std::endl;
	}
	SDL_Texture* medium_blue_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\spaceshipBlue.png");
	SDL_Texture* medium_yellow_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\spaceshipYellow.png");
	SDL_Texture* medium_red_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\spaceshipRed.png");
	SDL_Texture* medium_green_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\spaceshipGreen.png");
	SDL_Texture* medium_white_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\spaceshipWhite.png");
	//SDL_Texture* spaceship_low = IMG_LoadTexture(renderer, "..\\Project1\\assets\\spaceship_low.png");
	//SDL_Texture* spaceship_high = IMG_LoadTexture(renderer, "..\\Project1\\assets\\spaceship_high.png");
	//SDL_Texture* spaceship = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bear-idle.png");
	//SDL_Texture* spaceship_low = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bear-idle.png");
	//SDL_Texture* spaceship_high = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bear-idle.png");

	SDL_Texture* bullet_blue_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bulletBlue.png");
	SDL_Texture* bullet_yellow_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bulletYellow.png");
	SDL_Texture* bullet_red_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bulletRed.png");
	SDL_Texture* bullet_green_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bulletGreen.png");

	SDL_Texture* sun_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\sun.png");
	//SDL_Texture* bullet_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\bullet.png");
	SDL_Texture* missile_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\missle.png");
	SDL_Texture* explosion_tex = IMG_LoadTexture(renderer, "..\\Project1\\assets\\sun.png");
	SDL_Texture* cannon = IMG_LoadTexture(renderer, "..\\Project1\\assets\\cannon.png");
	SDL_Texture* right_arrow = IMG_LoadTexture(renderer, "..\\Project1\\assets\\right_arrow.png");
	SDL_Texture* left_arrow = IMG_LoadTexture(renderer, "..\\Project1\\assets\\left_arrow.png");

	TTF_Font* caladea36 = TTF_OpenFont("..\\Project1\\assets\\caladea-regular.ttf", 36); //this opens a font style and sets a size

	SDL_Event e;
	bool quit = false;
	SDL_Keycode k;
	int winner;

	// init game objects
	struct spaceship* ships[2];
	ships[0] = init_spaceship(300 * 10000, 300 * 10000, 1);
	ships[1] = init_spaceship(700 * 10000, 300 * 10000, 1);
	//ships[2] = init_spaceship(300*10000, 700*10000, 1);
	//ships[3] = init_spaceship(700*10000, 700*10000);

	Uint32 last_frame_start_time = SDL_GetTicks();
	Uint32 frame_start_time = SDL_GetTicks();

	const int frame_counter_size = 60;
	int frame_time[frame_counter_size + 1];
	int frame_counter = 0;

	bool is_fullscreen = false;
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
		//std::cout << frame_start_time - last_frame_start_time << std::endl;

		//INSERT MEGA IFS

		//start of main menu state
		if (currentState == mainMenu) {
			while (SDL_PollEvent(&e)) {
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
			int selection;
			int selections[4];


			while (SDL_PollEvent(&e)) {
				switch (e.type) {
				case SDL_CONTROLLERBUTTONDOWN:
					controller_index = e.cbutton.which;
					selection = selections[controller_index];
					if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
						currentState = stageSelect;
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
						currentState = mainMenu;
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {

					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {

					}
					break;
				}
			}

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
		

			// render ships 
			{
				//p1
				render_texture(medium_red_tex, renderer, 300, 150, 0, 4);
				render_texture(right_arrow, renderer, 400, 150, 0, 1);
				render_texture(left_arrow, renderer, 200, 150, 0, 1);


				//p2
				render_texture(medium_blue_tex, renderer, 1000, 150, 0, 4);
				render_texture(right_arrow, renderer, 1100, 150, 0, 1);
				render_texture(left_arrow, renderer, 900, 150, 0, 1);


				//p3
				render_texture(medium_yellow_tex, renderer, 300, 550, 0, 4);
				render_texture(right_arrow, renderer, 400, 550, 0, 1);
				render_texture(left_arrow, renderer, 200, 550, 0, 1);


				//p4
				render_texture(medium_green_tex, renderer, 1000, 550, 0, 4);
				render_texture(right_arrow, renderer, 1100, 550, 0, 1);
				render_texture(left_arrow, renderer, 900, 550, 0, 1);
			}

			SDL_RenderPresent(renderer);

		}
		// start of stage select state
		else if (currentState == stageSelect) {
			currentState = inGame;
			
		}
		// start of in game state
		else if (currentState == inGame) {
			int controller_index;
			struct spaceship* ship;

			SDL_RenderClear(renderer);


			// poll input
			{
				int controller_index;
				struct spaceship* ship;
				while (SDL_PollEvent(&e)) {
					switch (e.type) {
					case SDL_QUIT:
						quit = true;
						break;
					case SDL_KEYDOWN:
						k = e.key.keysym.sym;
						if (k == SDLK_f) {
							if (is_fullscreen) {
								SDL_SetWindowFullscreen(window, 0);
								SDL_ShowCursor(1);
							} else {
								SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
								SDL_ShowCursor(0);
							}
							is_fullscreen = !is_fullscreen;
						} else if (k == SDLK_ESCAPE) {
							if (is_fullscreen) {
								SDL_SetWindowFullscreen(window, 0);
								is_fullscreen = !is_fullscreen;
								SDL_ShowCursor(1);
							}
						}
						break;
					case SDL_CONTROLLERBUTTONDOWN:
						controller_index = e.cbutton.which;
						ship = ships[controller_index];
						if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_X) {

						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_Y) {

						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
							ship->fire_normal = true;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
							ship->fire_burst = true;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
							currentState = pause;
						} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
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
							std::cout << old_gun_mag << std::endl;
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
								ship->fire_missile = false;
							} else {
								ship->fire_missile = true;
							}
						} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
							int min_activation = 20000;
							//std::cout << e.caxis.value << std::endl;
							if (e.caxis.value < min_activation) {
								ship->fire_spread = false;
							} else {
								ship->fire_spread = true;
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
				struct spaceship* ship = ships[i];

				// regen stamina
				ship->stamina += SPACESHIP_STAMINA_PER_FRAME;
				if (ship->stamina > ship->stamina_max) {
					ship->stamina = ship->stamina_max;
				}

				// reduce invincibility time
				if (ship->invincibility_cooldown > 0) {
					ship->invincibility_cooldown--;
				}

				// handle normal bullet spawns
				if (ship->cannon_cooldown > 0) {
					ship->cannon_cooldown--;
				}
				if (ship->fire_normal && ship->stamina > 0 && ship->cannon_cooldown <= 0) {

					int MUZZLE_VEL = 60000;
					int spread = 1;
					bullet** new_bullets = spawn_bullets(ship, MUZZLE_VEL, spread, 5, 10, 400);
					for (int i = 0; i < spread; i++) {
						ship->bullets[ship->num_bullets] = new_bullets[i];
						ship->num_bullets++;
					}
					free(new_bullets);
					ship->cannon_cooldown += ship->cannon_delay;
					ship->stamina -= 100;
				}

				// handle burst fire spawns
				if (ship->burst_cooldown_1 > 0) {
					ship->burst_cooldown_1--;
				}
				if (ship->burst_shot_current != 0 && ship->burst_cooldown_2 > 0) {
					ship->burst_cooldown_2--;
				}
				if (ship->fire_burst && ship->stamina > 0 && ship->burst_cooldown_1 <= 0) {
					int MUZZLE_VEL = 90000;
					int spread = 1;
					bullet** new_bullets = spawn_bullets(ship, MUZZLE_VEL, spread, 3, 150, 0);
					for (int i = 0; i < spread; i++) {
						ship->bullets[ship->num_bullets] = new_bullets[i];
						ship->num_bullets++;
					}
					free(new_bullets);
					ship->burst_cooldown_1 += ship->burst_delay_1;
					ship->burst_shot_current++;
					ship->stamina -= 320;
				}
				if (ship->burst_cooldown_2 <= 0) {
					int MUZZLE_VEL = 90000;
					int spread = 1;
					bullet** new_bullets = spawn_bullets(ship, MUZZLE_VEL, spread, 3, 150, 0);
					for (int i = 0; i < spread; i++) {
						ship->bullets[ship->num_bullets] = new_bullets[i];
						ship->num_bullets++;
					}
					free(new_bullets);
					ship->burst_cooldown_2 += ship->burst_delay_2;
					ship->burst_shot_current++;
					if (ship->burst_shot_current == ship->burst_shot_number) {
						ship->burst_shot_current = 0;
					}
				}

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

				// handle missile spawns
				if (ship->missile_cooldown > 0) {
					ship->missile_cooldown--;
				}
				if (ship->fire_missile && ship->stamina > 0 && ship->missile_cooldown <= 0) {
					int MUZZLE_VEL = 50000;
					int spread = 1;
					missile** new_missiles = spawn_missiles(ship, MUZZLE_VEL, spread, 25, 200, 300);
					for (int i = 0; i < spread; i++) {
						ship->missiles[ship->num_missiles] = new_missiles[i];
						ship->num_missiles++;
					}
					free(new_missiles);
					ship->missile_cooldown += ship->missile_delay;
					ship->stamina -= 400;
				}

				// update ship itself
				{

					// update acceleration
					double accel_mag = 0.3*sqrt(pow(ship->move_dir_x, 2) + pow(ship->move_dir_y, 2));
					if (accel_mag > SPACESHIP_MAX_ACCEL) {
						accel_mag = SPACESHIP_MAX_ACCEL;
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
						ship->x_vel -= (int)((pow(ship->x_vel, 2) + SPACESHIP_STEADY_FRICTION) / (10000 * SPACESHIP_MAX_FRICTION));
						if (ship->x_vel < 0) {
							ship->x_vel = 0;
						}
					} else if (ship->x_vel < 0) {
						ship->x_vel += (int)((pow(ship->x_vel, 2) + SPACESHIP_STEADY_FRICTION) / (10000 * SPACESHIP_MAX_FRICTION));
						if (ship->x_vel > 0) {
							ship->x_vel = 0;
						}
					}

					if (ship->y_vel > 0) {
						ship->y_vel -= (int)((pow(ship->y_vel, 2) + SPACESHIP_STEADY_FRICTION) / (10000 * SPACESHIP_MAX_FRICTION));
						if (ship->y_vel < 0) {
							ship->y_vel = 0;
						}
					} else if (ship->y_vel < 0) {
						ship->y_vel += (int)((pow(ship->y_vel, 2) + SPACESHIP_STEADY_FRICTION) / (10000 * SPACESHIP_MAX_FRICTION));
						if (ship->y_vel > 0) {
							ship->y_vel = 0;
						}
					}


					// update position
					ship->x_pos += ship->x_vel;
					ship->y_pos += ship->y_vel;

					// handle death
					if (ship->x_pos < STATUS_BAR_WIDTH * 10000 || ship->x_pos > WINDOW_WIDTH * 10000 || ship->y_pos < 0 || ship->y_pos > WINDOW_HEIGHT * 10000) {
						ship->x_pos = 10000 * WINDOW_WIDTH / 2;
						ship->y_pos = 10000 * WINDOW_HEIGHT / 2;
						ship->x_vel = 0;
						ship->y_vel = 0;

						ship->percent = 0;
						ship->lives--;
						ship->invincibility_cooldown += ship->respawn_invincibility_delay;

						SDL_HapticRumblePlay(haptics[i], 1, 300);
					}

				}

				// update bullets
				for (int j = 0; j < ship->num_bullets; j++) {
					struct bullet* bullet = ship->bullets[j];

					bullet->x_vel += bullet->x_accel;
					bullet->y_vel += bullet->y_accel;

					bullet->x_pos += bullet->x_vel;
					bullet->y_pos += bullet->y_vel;
					//std::cout << ship->x_pos << std::endl;

					// check for bullet going out of bounds
					if (bullet->x_pos < STATUS_BAR_WIDTH * 10000 || bullet->x_pos > WINDOW_WIDTH * 10000 || bullet->y_pos < 0 || bullet->y_pos > WINDOW_HEIGHT * 10000) {
						ship->num_bullets--;
						free(ship->bullets[j]);
						ship->bullets[j] = ship->bullets[ship->num_bullets];
						j--;
						continue;
					}

					// check for collisions with enemies
					for (int k = 0; k < num_players; k++) {
						if (i == k) continue;
						double dist = sqrt(pow(bullet->x_pos - ships[k]->x_pos, 2) + pow(bullet->y_pos - ships[k]->y_pos, 2));
						//std::cout << dist << std::endl;
						if (dist <= (ship->radius + BULLET_RADIUS) * 10000) {
							if (ships[k]->invincibility_cooldown == 0) {
								// knockback
								int total_knockback = (int)((bullet->base_knockback + (ships[k]->percent / 100.0)*bullet->knockback_scaling) / ships[k]->weight);
								ships[k]->x_vel += (int)(1000.0*total_knockback*bullet->x_vel / sqrt(pow(bullet->x_vel, 2) + pow(bullet->y_vel, 2)));
								ships[k]->y_vel += (int)(1000.0*total_knockback*bullet->y_vel / sqrt(pow(bullet->x_vel, 2) + pow(bullet->y_vel, 2)));

								// damage
								ships[k]->percent += bullet->damage;
								if (ships[k]->percent > SPACESHIP_MAX_PERCENT) {
									ships[k]->percent = SPACESHIP_MAX_PERCENT;
								}

								// haptic
								float haptic_amount = 0.03f + total_knockback / 100.0f;
								if (haptic_amount > 1) {
									haptic_amount = 1;
								}
								SDL_HapticRumblePlay(haptics[k], haptic_amount, 160);
							}
							// delete bullet
							ship->num_bullets--;
							free(ship->bullets[j]);
							ship->bullets[j] = ship->bullets[ship->num_bullets];
							j--;
							break;
						}
					}
				}

				// update missiles
				for (int j = 0; j < ship->num_missiles; j++) {
					struct missile* missile = ship->missiles[j];

					if (!missile->exploded) {
						// find the closest enemy player. this is the one to lock on to.
						double min_dist = 99999999;
						int target_player = -1;
						for (int k = 0; k < num_players; k++) {
							if (i == k) continue;
							double dist = sqrt(pow(missile->x_pos - ships[k]->x_pos, 2) + pow(missile->y_pos - ships[k]->y_pos, 2));
							if (dist < min_dist) {
								min_dist = dist;
								target_player = k;
							}
						}

						// accelerate towards target player
						double delta_x = ships[target_player]->x_pos - missile->x_pos;
						double delta_y = ships[target_player]->y_pos - missile->y_pos;
						double delta_mag = sqrt(pow(delta_x, 2) + pow(delta_y, 2));

						missile->x_accel = MISSILE_ACCEL * delta_x / delta_mag;
						missile->y_accel = MISSILE_ACCEL * delta_y / delta_mag;

						missile->x_vel += missile->x_accel;
						missile->y_vel += missile->y_accel;

						missile->x_pos += missile->x_vel;
						missile->y_pos += missile->y_vel;
					}

					// check for missile going out of bounds
					if (missile->x_pos < STATUS_BAR_WIDTH * 10000 || missile->x_pos > WINDOW_WIDTH * 10000 || missile->y_pos < 0 || missile->y_pos > WINDOW_HEIGHT * 10000) {
						ship->num_missiles--;
						free(ship->missiles[j]);
						ship->missiles[j] = ship->missiles[ship->num_missiles];
						j--;
						continue;
					}

					// expand missile radius
					if (missile->exploded) {
						missile->radius += MISSILE_RADIUS_PER_FRAME;
						if (missile->radius > MISSILE_MAX_RADIUS) {
							ship->num_missiles--;
							free(ship->missiles[j]);
							ship->missiles[j] = ship->missiles[ship->num_missiles];
							j--;
							continue;
						}
					}

					// check for collisions with enemies
					for (int k = 0; k < num_players; k++) {
						//if (i == k) continue;

						double dist = sqrt(pow(missile->x_pos - ships[k]->x_pos, 2) + pow(missile->y_pos - ships[k]->y_pos, 2));
						if (!missile->exploded) {
							if (i == k) continue;
							if (dist <= MISSILE_ACTIVATION_RADIUS * 10000) {
								missile->exploded = true;
								missile->x_vel = 0;
								missile->y_vel = 0;
							}
						} else {
							if (dist <= (ship->radius + missile->radius) * 10000) {
								if (missile->players_hit[k]) continue;
								missile->players_hit[k] = true;
								if (ships[k]->invincibility_cooldown == 0) {

									// knockback
									int total_knockback = (int)((missile->base_knockback + (ships[k]->percent / 100.0)*missile->knockback_scaling) / ships[k]->weight);
									ships[k]->x_vel += (int)(1000.0*total_knockback*(ships[k]->x_pos - missile->x_pos) / sqrt(pow(missile->x_pos - ships[k]->x_pos, 2) + pow(missile->y_pos - ships[k]->y_pos, 2)));
									ships[k]->y_vel += (int)(1000.0*total_knockback*(ships[k]->y_pos - missile->y_pos) / sqrt(pow(missile->x_pos - ships[k]->x_pos, 2) + pow(missile->y_pos - ships[k]->y_pos, 2)));

									// damage
									ships[k]->percent += missile->damage;
									if (ships[k]->percent > SPACESHIP_MAX_PERCENT) {
										ships[k]->percent = SPACESHIP_MAX_PERCENT;
									}

									// haptic
									float haptic_amount = 0.03f + total_knockback / 100.0f;
									if (haptic_amount > 1) {
										haptic_amount = 1;
									}
									SDL_HapticRumblePlay(haptics[k], haptic_amount, 160);
								}
							}
						}
					}
				}

				//Check to see if the game is over
				int victoryCheck = 0;
				for (int i = 0; i < num_players; i++) {
					ship = ships[i];
					if (ship->lives == 0) {
						victoryCheck++;
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
				}

			} // end of update ships

			// begin rendering
			{
				SDL_RenderClear(renderer);

				// render background
				render_texture(bg, renderer, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0, 1);
				render_texture(sun_tex, renderer, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0, 1);

				// render all ship elements
				for (int i = 0; i < num_players; i++) {

					// choose textures
					SDL_Texture* ship_tex;
					SDL_Texture* bullet_tex;
					if (i == 0) {
						ship_tex = medium_red_tex;
						bullet_tex = bullet_red_tex;
					} else if (i == 1) {
						ship_tex = medium_blue_tex;
						bullet_tex = bullet_blue_tex;
					} else if (i == 2) {
						ship_tex = medium_yellow_tex;
						bullet_tex = bullet_yellow_tex;
					} else {
						ship_tex = medium_green_tex;
						bullet_tex = bullet_green_tex;
					}

					if (ships[i]->invincibility_cooldown > 0) {
						ship_tex = medium_white_tex;
					}


					// todo: relocate this
					if (!caladea36) {
						std::cout << TTF_GetError() << std::endl;
					}

					// render ship
					struct spaceship* ship = ships[i];
					double angle = calculate_angle(ship->face_dir_x, ship->face_dir_y);

					render_texture(ship_tex, renderer, ship->x_pos / 10000, ship->y_pos / 10000, angle, 3);

					// render gun
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
					for (int j = 0; j < ship->num_bullets; j++) {
						double angle = calculate_angle(ship->bullets[j]->x_vel, ship->bullets[j]->y_vel);
						//std::cout << bullets[i]->x_vel << "\t" << bullets[i]->y_vel << "\t" << angle << std::endl;
						render_texture(bullet_tex, renderer, ship->bullets[j]->x_pos / 10000, ship->bullets[j]->y_pos / 10000, angle, 1);
					}

					// render missiles
					for (int j = 0; j < ship->num_missiles; j++) {
						if (!ship->missiles[j]->exploded) {
							//std::cout << "rendering missile\n";
							double angle = calculate_angle(ship->missiles[j]->x_vel, ship->missiles[j]->y_vel);
							//std::cout << bullets[i]->x_vel << "\t" << bullets[i]->y_vel << "\t" << angle << std::endl;
							render_texture(missile_tex, renderer, ship->missiles[j]->x_pos / 10000, ship->missiles[j]->y_pos / 10000, angle, 1.8);
						} else {
							SDL_Rect rect;

							rect.w = ship->missiles[j]->radius * 2;
							rect.h = ship->missiles[j]->radius * 2;
							rect.x = ship->missiles[j]->x_pos / 10000 - rect.w / 2;
							rect.y = ship->missiles[j]->y_pos / 10000 - rect.h / 2;

							SDL_RenderCopyEx(renderer, explosion_tex, NULL, &rect, 0, NULL, SDL_FLIP_NONE);
						}

					}
					

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
						struct spaceship* ship = ships[i];

						// render stamina bar
						{
							SDL_SetRenderDrawColor(renderer, 0, 160, 0, SDL_ALPHA_OPAQUE);
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
				switch (e.type) {
				case SDL_CONTROLLERBUTTONDOWN:
					if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
						currentState = mainMenu;
					}
					break;
				}
			}
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
	// todo: destroy other textures
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}

bullet** spawn_bullets(spaceship* ship, int velocity, int spread, int damage, int base_knockback, int knockback_scaling) {
	double spread_angle = 3.14159 * 0.1;

	bullet** new_bullets = (bullet**)malloc(sizeof(bullet*) * spread);

	if (spread % 2 == 0) {

	} else {
		double straight_theta = atan2(ship->gun_dir_y, ship->gun_dir_x);


		for (int i = 0; i < spread; i++) {
			double theta;
			if (i % 2 == 0) {
				theta = straight_theta + (i / 2)*spread_angle;
			} else {
				theta = straight_theta - ((i + 1) / 2)*spread_angle;
			}

			int x_vel = (int)(velocity*cos(theta));
			int y_vel = (int)(velocity*sin(theta));

			if (x_vel != 0 || y_vel != 0) {
				new_bullets[i] = init_bullet(ship->x_pos, ship->y_pos, x_vel, y_vel, damage, base_knockback, knockback_scaling);
			}
		}
	}

	return new_bullets;
	
}

missile** spawn_missiles(spaceship* ship, int velocity, int spread, int damage, int base_knockback, int knockback_scaling) {


	double spread_angle = 3.14159 * 0.2;

	missile** new_missiles = (missile**)malloc(sizeof(missile*) * spread);

	if (spread % 2 == 0) {

	} else {
		double straight_theta = atan2(ship->gun_dir_y, ship->gun_dir_x);


		for (int i = 0; i < spread; i++) {
			double theta;
			if (i % 2 == 0) {
				theta = straight_theta + (i / 2)*spread_angle;
			} else {
				theta = straight_theta - ((i + 1) / 2)*spread_angle;
			}

			int x_vel = (int)(velocity*cos(theta));
			int y_vel = (int)(velocity*sin(theta));

			if (x_vel != 0 || y_vel != 0) {
				new_missiles[i] = init_missile(ship->x_pos, ship->y_pos, x_vel, y_vel, damage, base_knockback, knockback_scaling);
			}
		}
	}

	return new_missiles;

}

double calculate_angle(int x_vel, int y_vel) {
	double angle;
	double conversion = 180 / M_PI;
	angle = atan2(y_vel, x_vel) * conversion + 90;
	return angle;
}

void render_texture(SDL_Texture* texture, SDL_Renderer* renderer, int x, int y, double angle, double scaling) {
	SDL_Rect rect;
	
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	rect.w *= scaling;
	rect.h *= scaling;
	rect.x = x-rect.w/2;
	rect.y = y-rect.h/2;

	SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
}
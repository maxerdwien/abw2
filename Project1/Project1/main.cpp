#include <iostream>
#include <string>
#include <math.h>

#include <stdint.h>
#include "platform-compat.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <time.h>

#include "serializer.h"

#include "input-state.h"

#include "spaceship.h"
#include "grizzly.h"
#include "black.h"
#include "polar.h"
#include "panda.h"
#include "kodiak.h"

#include "item.h"

#include "asteroid.h"

#include "stage_select_marker.h"
#include "stage_marker.h"
#include "wind.h"

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
const int ASTEROID_SPAWN_DIST = 10000 * 100;
const int BARSIZE = 10000 * 26;
const int STATUS_BAR_WIDTH = 10000 * 150;

bool dpad_down[4] = { false, false, false, false };

Renderer* r;

// debug conveniences (skip menus/countdowns, windowed, muted, fps in title).
// on only in debug builds: visual studio's Debug configuration defines _DEBUG.
#ifdef _DEBUG
const bool DEBUG_MODE = true;
#else
const bool DEBUG_MODE = false;
#endif

bool quit = false;
bool is_fullscreen = !DEBUG_MODE;
bool xp_mode = false;
bool muted = DEBUG_MODE;

int controller_mappings[4] = { -1, -1, -1, -1 };
Input_State controllers[4];
SDL_Haptic* haptics[4];
lra LRA[4];

Mix_Music* music;

bool read_global_input(SDL_Event* e);
void read_input();

void log_joystick_devices(const char* when);
void log_controller_diagnostics();

enum class ship_type {
	black = 0,
	grizzly = 1,
	polar = 2,
	panda = 3,
	kodiak = 4,

	MAX,
};

enum class stage {
	anchorage, // normal
	fairbanks, // asteroid in middle
	juneau, // random asteroids
	nome, // wind gusts
	utqiagvik, // drifting ice floes

	MAX,
};

enum team_mode {
	free_for_all,
	two_vs_two,
};

team_mode selected_team_mode = free_for_all;

stage selected_stage = stage::anchorage;

const double CONTROLLER_MAX_ANGLE = M_PI / 6;
const int DEAD_ZONE = 5000;

const int render_data_buffer_size = 4096;
char render_data_buffer[render_data_buffer_size];

// todo: add warnings when approaching this buffer size
const int input_data_buffer_size = 256;
char input_data_buffer[input_data_buffer_size];

SOCKET them;
SOCKET me;

Ship* ships[4] = { NULL, NULL, NULL, NULL };

Item* items[100];
int num_items = 0;

Asteroid* asteroids[100];
int num_asteroids = 0;

// wind gusts (nome stage)
Wind* wind = NULL;

int lookup_controller(int instanceID);
void render_character_selector(int x, int y, SDL_Texture* ship_tex, ship_type shipType, SDL_Texture* right_arrow, SDL_Texture* left_arrow, bool ready, player_status ps, bool has_controller);
void render_results(int x, int y, SDL_Texture * ship_tex, Ship * ship);
void render_game(int game_end_cooldown, int game_end_delay, int game_start_cooldown,
	Mix_Chunk* countdown_tick, Mix_Chunk* selected_ship, SDL_Texture* bg,
	Ship* ships[], Item* items[], int num_items, Asteroid* asteroids[], int num_asteroids);

SOCKET connect_to_ip(PCSTR ip_address);
SOCKET get_local_socket();

void send_buffer(SOCKET them, char* buf, int size);
void get_buffer(SOCKET me, char* buf, int size);

void get_input_data_forever(void* ptr);
void get_render_data_forever(void* ptr);

ship_type increment_ship_selection(ship_type st, int amount);

void toggle_mute();

int main(int, char**) {
#ifndef _WIN32
	// assets are loaded relative to the executable ("../Project1/assets/..."), so make
	// sure the working directory is the executable's directory regardless of how we were launched
	if (char* base_path = SDL_GetBasePath()) {
		if (chdir(base_path) != 0) {
			printf("could not chdir to %s\n", base_path);
		}
		SDL_free(base_path);
	}
#endif

	enum class online_status {
		local,
		host,
		client,
	};

	enum game_state {
		main_menu,
		options,
		character_select,
		stage_select,
		init_game,
		in_game,
		results,
		pause,
	};

	enum color {
		red = 0,
		blue = 1,
		yellow = 2,
		green = 3,
		teal = 4,
		magenta = 5,
	};

	game_state current_state = main_menu;
	if (DEBUG_MODE) {
		current_state = character_select;
	}

	// init SDL
	// enable SDL's HIDAPI driver so the official nintendo gamecube usb adapter (WUP-028) is picked up
	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_GAMECUBE, "1");
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cout << "SDL_Init error: " << SDL_GetError() << std::endl;
		return 1;
	}

	log_controller_diagnostics();

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
		_2160p, // 4k
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

	window = SDL_CreateWindow("alaskan cosmobear spacefighting", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (!window) {
		std::cout << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		SDL_DestroyWindow(window);
		std::cout << "SDL_CreateRenderer error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	//SDL_Texture* rend_tex = malloc(sizeof(class SDL_Texture));
	//SDL_SetRenderTarget(renderer, rend_tex);

	const int game_start_delay = 4 * 60 + 3;
	int game_start_cooldown;

	const int game_end_delay = 2 * 60;
	int game_end_cooldown;

	const int playerUiWidth = 10;

	r = new Renderer(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

	// load textures
	SDL_Texture* bg = r->LoadTexture("../Project1/assets/background.png");

	SDL_Texture* ship_textures[(int)ship_type::MAX][6];
	ship_textures[(int)ship_type::black][red] = r->LoadTexture("../Project1/assets/ships/black-red.png");
	ship_textures[(int)ship_type::black][blue] = r->LoadTexture("../Project1/assets/ships/black-blue.png");
	ship_textures[(int)ship_type::black][yellow] = r->LoadTexture("../Project1/assets/ships/black-yellow.png");
	ship_textures[(int)ship_type::black][green] = r->LoadTexture("../Project1/assets/ships/black-green.png");
	ship_textures[(int)ship_type::black][teal] = r->LoadTexture("../Project1/assets/ships/black-teal.png");
	ship_textures[(int)ship_type::black][magenta] = r->LoadTexture("../Project1/assets/ships/black-magenta.png");

	ship_textures[(int)ship_type::grizzly][red] = r->LoadTexture("../Project1/assets/ships/grizzly-red.png");
	ship_textures[(int)ship_type::grizzly][blue] = r->LoadTexture("../Project1/assets/ships/grizzly-blue.png");
	ship_textures[(int)ship_type::grizzly][yellow] = r->LoadTexture("../Project1/assets/ships/grizzly-yellow.png");
	ship_textures[(int)ship_type::grizzly][green] = r->LoadTexture("../Project1/assets/ships/grizzly-green.png");
	ship_textures[(int)ship_type::grizzly][teal] = r->LoadTexture("../Project1/assets/ships/grizzly-teal.png");
	ship_textures[(int)ship_type::grizzly][magenta] = r->LoadTexture("../Project1/assets/ships/grizzly-magenta.png");

	ship_textures[(int)ship_type::polar][red] = r->LoadTexture("../Project1/assets/ships/polar-red.png");
	ship_textures[(int)ship_type::polar][blue] = r->LoadTexture("../Project1/assets/ships/polar-blue.png");
	ship_textures[(int)ship_type::polar][yellow] = r->LoadTexture("../Project1/assets/ships/polar-yellow.png");
	ship_textures[(int)ship_type::polar][green] = r->LoadTexture("../Project1/assets/ships/polar-green.png");
	ship_textures[(int)ship_type::polar][teal] = r->LoadTexture("../Project1/assets/ships/polar-teal.png");
	ship_textures[(int)ship_type::polar][magenta] = r->LoadTexture("../Project1/assets/ships/polar-magenta.png");

	ship_textures[(int)ship_type::panda][red] = r->LoadTexture("../Project1/assets/ships/panda-red.png");
	ship_textures[(int)ship_type::panda][blue] = r->LoadTexture("../Project1/assets/ships/panda-blue.png");
	ship_textures[(int)ship_type::panda][yellow] = r->LoadTexture("../Project1/assets/ships/panda-yellow.png");
	ship_textures[(int)ship_type::panda][green] = r->LoadTexture("../Project1/assets/ships/panda-green.png");
	ship_textures[(int)ship_type::panda][teal] = r->LoadTexture("../Project1/assets/ships/panda-teal.png");
	ship_textures[(int)ship_type::panda][magenta] = r->LoadTexture("../Project1/assets/ships/panda-magenta.png");

	ship_textures[(int)ship_type::kodiak][red] = r->LoadTexture("../Project1/assets/ships/kodiak-red.png");
	ship_textures[(int)ship_type::kodiak][blue] = r->LoadTexture("../Project1/assets/ships/kodiak-blue.png");
	ship_textures[(int)ship_type::kodiak][yellow] = r->LoadTexture("../Project1/assets/ships/kodiak-yellow.png");
	ship_textures[(int)ship_type::kodiak][green] = r->LoadTexture("../Project1/assets/ships/kodiak-green.png");
	ship_textures[(int)ship_type::kodiak][teal] = r->LoadTexture("../Project1/assets/ships/kodiak-teal.png");
	ship_textures[(int)ship_type::kodiak][magenta] = r->LoadTexture("../Project1/assets/ships/kodiak-magenta.png");

	SDL_Texture* right_arrow = r->LoadTexture("../Project1/assets/right_arrow.png");
	SDL_Texture* left_arrow = r->LoadTexture("../Project1/assets/left_arrow.png");

	SDL_Texture* alaska = r->LoadTexture("../Project1/assets/alaska.png");

	
	Mix_Chunk* beep = Mix_LoadWAV("../Project1/assets/sounds/confirm.wav");
	Mix_Chunk* deselect = Mix_LoadWAV("../Project1/assets/sounds/deselect.wav");
	Mix_Chunk* selected_ship = Mix_LoadWAV("../Project1/assets/sounds/beep.wav");
	Mix_Chunk* powerup_sfx = Mix_LoadWAV("../Project1/assets/sounds/item.wav");
	Mix_Chunk* death_sfx = Mix_LoadWAV("../Project1/assets/sounds/death.wav");
	Mix_Chunk* countdown_tick = Mix_LoadWAV("../Project1/assets/sounds/countdown.wav");

	wind = new Wind(r, STATUS_BAR_WIDTH, WIDTH_UNITS, 0, HEIGHT_UNITS);


	music = Mix_LoadMUS("../Project1/assets/sounds/Cyborg_Ninja.wav");
	// "Cyborg Ninja" Kevin MacLeod (incompetech.com)
	// Licensed under Creative Commons : By Attribution 3.0 License
	// http ://creativecommons.org/licenses/by/3.0/
	if (!muted) {
		Mix_PlayMusic(music, -1);
		Mix_AllocateChannels(32);
	} else {
		Mix_AllocateChannels(0);
	}

	Mix_VolumeMusic(MIX_MAX_VOLUME / 6);

	const int min_analog_to_dpad_angle = 26000;


	// stage stuff
	int asteroid_spawn_cooldown = 1;

	//ship_type selections[4] = { ship_type::grizzly, ship_type::grizzly, ship_type::grizzly, ship_type::grizzly };
	ship_type selections[4] = { ship_type::kodiak, ship_type::grizzly, ship_type::grizzly, ship_type::grizzly };
	bool analog_stick_moved[4] = { false, false, false, false };
	bool ready[4] = { false, false, false, false };


	bool do_items = false;

	SDL_Event e;
	
	int winner;
	

	int item_spawn_cooldown = 60 * 17;
	//if (DEBUG_MODE) item_spawn_cooldown = 2;

	int stock_count = 4;

	// game logic is tuned for 60 fps, so cap the frame rate there instead of vsync
	const double target_frame_ms = 1000.0 / 60.0;
	double next_frame_time = SDL_GetTicks();

	// fps counter: one bucket per second, title shows the average of the last 5
	const int fps_bucket_count = 5;
	int fps_frame_buckets[fps_bucket_count] = {};
	Uint32 fps_ms_buckets[fps_bucket_count] = {};
	int fps_bucket_index = 0;
	Uint32 fps_bucket_start = SDL_GetTicks();

	enum class main_menu_option : int {
		local,
		client,
		host,
		exit,

		MAX
	};

	main_menu_option main_menu_selection = main_menu_option::local;

	
	if (is_fullscreen) {
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
		SDL_ShowCursor(0);
	}

	// internet junk
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("wsa startup error, %d\n", WSAGetLastError());
	}

	const int handshake_buffer_size = 128;
	char handshake_buffer[handshake_buffer_size];

	// todo: memset all buffers?
	//memset(render_data_buffer, 0, render_data_buffer_size);
	memset(handshake_buffer, 0, handshake_buffer_size);


	// stage select marker
	Stage_Select_Marker* ssm = new Stage_Select_Marker(r);

	Stage_Marker* stage_markers[(int)stage::MAX];
	stage_markers[(int)stage::anchorage] = new Stage_Marker(r, 6872830, 4379670, " anchorage ", " plain ");
	stage_markers[(int)stage::fairbanks] = new Stage_Marker(r, 7474640, 3144430, " fairbanks ", " central asteroid ");
	stage_markers[(int)stage::juneau] = new Stage_Marker(r, 11169940, 5735230, " juneau ", " random asteroids ");
	stage_markers[(int)stage::nome] = new Stage_Marker(r, 3200000, 3500000, " nome ", " wind gusts ");
	stage_markers[(int)stage::utqiagvik] = new Stage_Marker(r, 5850000, 220000, " utqiagvik ", " drifting ice ");

	online_status os = online_status::local;

	/*int x = SDL_NumJoysticks();

	SDL_Joystick* js = SDL_JoystickOpen(0);
	SDL_JoystickGUID guid = SDL_JoystickGetGUID(js);

	char guid_str[1024];
	SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
	const char* name = SDL_JoystickName(js);

	int num_axes = SDL_JoystickNumAxes(js);
	int num_buttons = SDL_JoystickNumButtons(js);
	int num_hats = SDL_JoystickNumHats(js);
	int num_balls = SDL_JoystickNumBalls(js);

	printf("%s \"%s\" axes:%d buttons:%d hats:%d balls:%d\n",
		guid_str, name,
		num_axes, num_buttons, num_hats, num_balls);

	SDL_GameControllerAddMappingsFromFile("..\\Project1\max-sdl-controller-mappings.txt");*/

	// game loop
	while (!quit) {
		next_frame_time += target_frame_ms;
		Uint32 frame_now = SDL_GetTicks();
		if (frame_now < next_frame_time) {
			SDL_Delay((Uint32)(next_frame_time - frame_now));
		} else if (frame_now - next_frame_time > 100.0) {
			// running way behind; skip ahead instead of fast-forwarding to catch up
			next_frame_time = frame_now;
		}

		if (DEBUG_MODE) {
			fps_frame_buckets[fps_bucket_index]++;
			Uint32 now = SDL_GetTicks();
			Uint32 elapsed = now - fps_bucket_start;
			if (elapsed >= 1000) {
				fps_ms_buckets[fps_bucket_index] = elapsed;
				int total_frames = 0;
				Uint32 total_ms = 0;
				for (int i = 0; i < fps_bucket_count; i++) {
					total_frames += fps_frame_buckets[i];
					total_ms += fps_ms_buckets[i];
				}
				char title[80];
				snprintf(title, sizeof(title), "alaskan cosmobear spacefighting - %.1f fps", total_frames * 1000.0 / total_ms);
				SDL_SetWindowTitle(window, title);
				fps_bucket_index = (fps_bucket_index + 1) % fps_bucket_count;
				fps_frame_buckets[fps_bucket_index] = 0;
				fps_bucket_start = now;
			}
		}

		// start of main menu state
		if (current_state == main_menu) {
			while (SDL_PollEvent(&e)) {
				if (read_global_input(&e)) continue;
				switch (e.type) {
				case SDL_CONTROLLERBUTTONDOWN: {
					if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A || e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
						Mix_PlayChannel(-1, beep, 0);

						switch (main_menu_selection) {
						case main_menu_option::local:
							os = online_status::local;
							break;
						case main_menu_option::exit:
							quit = true;
							break;
						case main_menu_option::host: {
							os = online_status::host;
							me = get_local_socket();

							char name[80];
							if (gethostname(name, sizeof(name)) == SOCKET_ERROR) {
								printf("problemo");
							}

							SOCKADDR_STORAGE from;
							socklen_t fromlen = sizeof(from);
							int message_len = recvfrom(me, handshake_buffer, handshake_buffer_size, 0, (LPSOCKADDR)&from, &fromlen);
							if (message_len == SOCKET_ERROR) {
								printf("recvfrom failed, %d", WSAGetLastError());
							}

							printf("recieved %d bytes\n", message_len);
							printf("%s\n", handshake_buffer);

							char hostname[NI_MAXHOST];
							// todo: worry about return value
							int ret_val = getnameinfo((LPSOCKADDR)&from, fromlen, hostname, sizeof(hostname), NULL, 0, NI_NUMERICHOST);
							if (ret_val != 0) {
								printf("could not get name info\n");
							}

							them = connect_to_ip(hostname);

							char message[render_data_buffer_size] = "hello world yourself";
							send_buffer(them, message, handshake_buffer_size);
							break;
						}
						case main_menu_option::client: {
							os = online_status::client;
							them = connect_to_ip("2601:282:a03:9e30:7812:e4af:d650:e3ee"); // hermione's ip address
							//them = connect_to_ip("2601:282:a03:9e30:49a2:8b4c:9b3b:96a9"); // greg's ip address

							char message[handshake_buffer_size] = "hello world";

							send_buffer(them, message, handshake_buffer_size);

							memset(handshake_buffer, 0, sizeof(handshake_buffer));

							me = get_local_socket();

							get_buffer(me, handshake_buffer, handshake_buffer_size);
							printf("%s\n", handshake_buffer);
							break;
						}
						}
						current_state = character_select;

					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
						Mix_PlayChannel(-1, beep, 0);
						int new_selection = (int)main_menu_selection - 1;
						if (new_selection < 0) new_selection += (int)main_menu_option::MAX;
						main_menu_selection = (main_menu_option)new_selection;
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
						Mix_PlayChannel(-1, beep, 0);
						int new_selection = (int)main_menu_selection + 1;
						if (new_selection == (int)main_menu_option::MAX) new_selection = 0;
						main_menu_selection = (main_menu_option)new_selection;
					}
					break;
				}
				case SDL_CONTROLLERAXISMOTION: {
					int controller_index = lookup_controller(e.caxis.which);
					if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {

						if (e.caxis.value > min_analog_to_dpad_angle && !analog_stick_moved[controller_index]) {
							analog_stick_moved[controller_index] = true;
							Mix_PlayChannel(-1, beep, 0);
							int new_selection = (int)main_menu_selection + 1;
							if (new_selection == (int)main_menu_option::MAX) new_selection = 0;
							main_menu_selection = (main_menu_option)new_selection;
						}
						else if (e.caxis.value < -min_analog_to_dpad_angle && !analog_stick_moved[controller_index]) {
							analog_stick_moved[controller_index] = true;
							Mix_PlayChannel(-1, beep, 0);
							int new_selection = (int)main_menu_selection - 1;
							if (new_selection < 0) new_selection += (int)main_menu_option::MAX;
							main_menu_selection = (main_menu_option)new_selection;
						}
						else if (e.caxis.value < min_analog_to_dpad_angle && e.caxis.value > -min_analog_to_dpad_angle) {
							analog_stick_moved[controller_index] = false;
						}
					}
					break;
				}
				}
			}

			// render basic black background
			r->render_solid_bg();

			// render title name and prompt to move forward
			{
				r->render_text(WIDTH_UNITS / 2, 2 * HEIGHT_UNITS / 10, "alaskan cosmobear spacefighting", true, false, false, large_f, 255, 255);

				r->render_text(WIDTH_UNITS / 2, 4 * HEIGHT_UNITS / 10, "start local game", true, false, main_menu_selection == main_menu_option::local, medium_f, 255, 255);
				r->render_text(WIDTH_UNITS / 2, 5 * HEIGHT_UNITS / 10, "look for online game", true, false, main_menu_selection == main_menu_option::client, medium_f, 255, 255);
				r->render_text(WIDTH_UNITS / 2, 6 * HEIGHT_UNITS / 10, "host online game", true, false, main_menu_selection == main_menu_option::host, medium_f, 255, 255);
				r->render_text(WIDTH_UNITS / 2, 7 * HEIGHT_UNITS / 10, "exit", true, false, main_menu_selection == main_menu_option::exit, medium_f, 255, 255);
			}

			SDL_RenderPresent(renderer);

		}
		// start of character select state
		else if (current_state == character_select) {

			int controller_index;

			while (SDL_PollEvent(&e)) {
				if (read_global_input(&e)) continue;
				switch (e.type) {
				case SDL_CONTROLLERBUTTONDOWN:
					controller_index = lookup_controller(e.cbutton.which);
					
					if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {

						if (!ready[controller_index]) {
							Mix_PlayChannel(-1, selected_ship, 0);
						}
						ready[controller_index] = true;
						
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
						bool all_ready = true;
						for (int i = 0; i < 4; i++) {
							if (controllers[i].status == human && !ready[i]) {
								all_ready = false;
							}
						}

						if (DEBUG_MODE) all_ready = true;
						if (all_ready) {
							ssm->x_pos = stage_markers[(int)stage::anchorage]->x_pos;
							ssm->y_pos = stage_markers[(int)stage::anchorage]->y_pos;
							current_state = stage_select;
						}
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
						Mix_PlayChannel(-1, deselect, 0);
						if (ready[controller_index] == false) {
							current_state = main_menu;
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
						selections[controller_index] = increment_ship_selection(selections[controller_index], 1);
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
						if (ready[controller_index]) break;
						Mix_PlayChannel(-1, beep, 0);
						selections[controller_index] = increment_ship_selection(selections[controller_index], -1);
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
						//if (ready[controller_index]) break;
						Mix_PlayChannel(-1, beep, 0);

						if (controllers[controller_index].status == human) {
							controllers[controller_index].status = ai;
							ready[controller_index] = true;
						} else if (controllers[controller_index].status == ai) {
							controllers[controller_index].status = human;
						}

						//int random_selection = rand() % 3;
						//selections[controller_index] = (ship_type)random_selection;
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_X) {
						// add an ai player to the first slot with no controller plugged in.
						// if every slot is taken, clear out all of the controller-less ai players instead.
						int empty_slot = -1;
						for (int i = 0; i < 4; i++) {
							if (controllers[i].status == empty && controller_mappings[i] == -1) {
								empty_slot = i;
								break;
							}
						}
						if (empty_slot != -1) {
							Mix_PlayChannel(-1, beep, 0);
							controllers[empty_slot].status = ai;
							selections[empty_slot] = (ship_type)(rand() % (int)ship_type::MAX);
							ready[empty_slot] = true;
						} else {
							bool removed_any = false;
							for (int i = 0; i < 4; i++) {
								if (controllers[i].status == ai && controller_mappings[i] == -1) {
									controllers[i] = Input_State();
									ready[i] = false;
									removed_any = true;
								}
							}
							if (removed_any) Mix_PlayChannel(-1, deselect, 0);
						}
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
						if (selected_team_mode == free_for_all) {
							selected_team_mode = two_vs_two;
						} else if (selected_team_mode == two_vs_two) {
							selected_team_mode = free_for_all;
						}
						Mix_PlayChannel(-1, beep, 0);
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
						stock_count++;
						if (stock_count > 99) stock_count -= 99;
						Mix_PlayChannel(-1, beep, 0);

					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
						stock_count--;
						if (stock_count < 1) stock_count += 99;
						Mix_PlayChannel(-1, deselect, 0);

					}
					break;
				case SDL_CONTROLLERAXISMOTION:
					controller_index = lookup_controller(e.caxis.which);
					if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {

						if (e.caxis.value > min_analog_to_dpad_angle && !analog_stick_moved[controller_index]) {
							if (ready[controller_index]) break;
							analog_stick_moved[controller_index] = true;
							Mix_PlayChannel(-1, beep, 0);
							selections[controller_index] = increment_ship_selection(selections[controller_index], 1);
						} else if (e.caxis.value < -min_analog_to_dpad_angle && !analog_stick_moved[controller_index]) {
							if (ready[controller_index]) break;
							analog_stick_moved[controller_index] = true;
							Mix_PlayChannel(-1, beep, 0);
							selections[controller_index] = increment_ship_selection(selections[controller_index], -1);
						} else if (e.caxis.value < min_analog_to_dpad_angle && e.caxis.value > -min_analog_to_dpad_angle) {
							analog_stick_moved[controller_index] = false;
						}
					}
					break;
				}
			}

			// render black background
			r->render_solid_bg();

			// render item toggle
			if (do_items) {
				r->render_text(0, 0, " Items: ON (Y) ", false, false, true, small_f, 255, 255);
			} else {
				r->render_text(0, 0, " Items: OFF (Y) ", false, false, true, small_f, 255, 255);
			}

			// render add/clear ai prompt
			{
				bool has_empty_slot = false;
				bool has_controllerless_ai = false;
				for (int i = 0; i < 4; i++) {
					if (controllers[i].status == empty && controller_mappings[i] == -1) has_empty_slot = true;
					if (controllers[i].status == ai && controller_mappings[i] == -1) has_controllerless_ai = true;
				}
				if (has_empty_slot) {
					r->render_text((WIDTH_UNITS + BARSIZE) / 2, 0, " Add AI Player (X) ", false, false, true, small_f, 255, 255);
				} else if (has_controllerless_ai) {
					r->render_text((WIDTH_UNITS + BARSIZE) / 2, 0, " Clear AI Players (X) ", false, false, true, small_f, 255, 255);
				}
			}

			// render team mode
			if (selected_team_mode == free_for_all) {
				r->render_text(0, (HEIGHT_UNITS + BARSIZE) / 2, " Mode: Free for All (RB) ", false, false, true, small_f, 255, 255);
			} else  if (selected_team_mode == two_vs_two) {
				r->render_text(0, (HEIGHT_UNITS + BARSIZE) / 2, " Mode: Two vs Two (RB) ", false, false, true, small_f, 255, 255);
			}

			// render number of stocks
			char stocks[20];
			snprintf(stocks, 20, " Stocks: %d (DPAD) ", stock_count);
			r->render_text((WIDTH_UNITS + BARSIZE) / 2, (HEIGHT_UNITS + BARSIZE) / 2, stocks, false, false, true, small_f, 255, 255);

			r->render_cross_bars(BARSIZE);
			
			// render ships selections
			{
				color c = red;
				render_character_selector(0, 0, ship_textures[(int)selections[0]][c], selections[0], right_arrow, left_arrow, ready[0], controllers[0].status, controller_mappings[0] != -1);

				if (selected_team_mode == two_vs_two) c = magenta;
				else c = blue;
				render_character_selector(WIDTH_UNITS / 2 + BARSIZE / 2, 0, ship_textures[(int)selections[1]][c], selections[1], right_arrow, left_arrow, ready[1], controllers[1].status, controller_mappings[1] != -1);

				if (selected_team_mode == two_vs_two) c = blue;
				else c = yellow;
				render_character_selector(0, HEIGHT_UNITS / 2 + BARSIZE / 2, ship_textures[(int)selections[2]][c], selections[2], right_arrow, left_arrow, ready[2], controllers[2].status, controller_mappings[2] != -1);

				if (selected_team_mode == two_vs_two) c = teal;
				else c = green;
				render_character_selector(WIDTH_UNITS / 2 + BARSIZE / 2, HEIGHT_UNITS / 2 + BARSIZE / 2, ship_textures[(int)selections[3]][c], selections[3], right_arrow, left_arrow, ready[3], controllers[3].status, controller_mappings[3] != -1);

			}

			// render "press start to begin"
			bool all_ready = true;
			for (int i = 0; i < 4; i++) {
				if (controllers[i].status == human && !ready[i]) {
					all_ready = false;
				}
			}
			if (all_ready) {
				r->render_text(WIDTH_UNITS / 2, HEIGHT_UNITS / 2, " Press Start to begin! ", true, true, true, large_f, 255, 255);
			}

			SDL_RenderPresent(renderer);

		}
		// start of stage select state
		else if (current_state == stage_select) {
			read_input();

			int total_x_motion = 0;
			int total_y_motion = 0;

			bool real_players_exist = false;
			for (int i = 0; i < 4; i++) {
				Input_State is = controllers[i];
				if (is.status == human) {
					real_players_exist = true;
				}
			}

			if (!real_players_exist) {
				current_state = init_game;
				selected_stage = stage::anchorage;
			}

			for (int i = 0; i < 4; i++) {
				Input_State is = controllers[i];
				if (is.status == empty) continue;

				if ((is.a.changed && is.a.state) || (is.start.changed && is.start.state)) {
					Mix_PlayChannel(-1, beep, 0);
					current_state = init_game;
				}

				if (is.b.changed && is.b.state) {
					Mix_PlayChannel(-1, beep, 0);
					current_state = character_select;
				}

				if (is.y.changed) {
					printf("%d %d\n", ssm->x_pos, ssm->y_pos);
				}

				double left_stick_mag = sqrt(pow(is.l_stick_x, 2) + pow(is.l_stick_y, 2));
				if (left_stick_mag > DEAD_ZONE) {
					total_x_motion += is.l_stick_x;
					total_y_motion += is.l_stick_y;
				}
			}

			ssm->x_pos += total_x_motion * 10;
			ssm->y_pos += total_y_motion * 10;

			for (int i = 0; i < (int)stage::MAX; i++) {
				double dist = sqrt(pow(stage_markers[i]->x_pos - ssm->x_pos, 2) + pow(stage_markers[i]->y_pos - ssm->y_pos, 2));

				const int attraction_speed = 1000;

				if (dist <= stage_markers[i]->attraction_radius) {
					double force = 5000000000000000.0 / pow(dist, 2);
					if (force > 60000) force = 60000;
					if (force < 20000) force = 20000;
					double angle = atan2(stage_markers[i]->y_pos - ssm->y_pos, stage_markers[i]->x_pos - ssm->x_pos);

					int delta_x = (int)(force * cos(angle));
					int delta_y = (int)(force * sin(angle));
					if (dist < sqrt(pow(delta_x, 2) + pow(delta_y, 2))) {
						ssm->x_pos = stage_markers[i]->x_pos;
						ssm->y_pos = stage_markers[i]->y_pos;
					} else {
						ssm->x_pos += delta_x;
						ssm->y_pos += delta_y;
					}
				}

				// multiple stage markers may have overlapping selection radii.
				// in this case, the last stage marker in the list will be selected.
				if (dist <= stage_markers[i]->selection_radius) {
					stage_markers[i]->selected = true;
					selected_stage = (stage)i;
				} else {
					stage_markers[i]->selected = false;
				}
			}


			r->render_solid_bg();
			r->render_background_image(alaska);

			
			ssm->render();

			for (int i = 0; i < (int)stage::MAX; i++) {
				stage_markers[i]->render();
			}
			

			SDL_RenderPresent(renderer);
			
		}
		else if (current_state == init_game) {
			// begin start game timer
			if (!DEBUG_MODE) {
				game_start_cooldown = game_start_delay;
			} else {
				game_start_cooldown = 0;
			}
			game_end_cooldown = game_end_delay;

			// todo: don't start new threads for every game
			// start threads to listen for online input
			if (os == online_status::host) {
				_beginthread(get_input_data_forever, 0, 0);
			}
			else if (os == online_status::client) {
				_beginthread(get_render_data_forever, 0, 0);
			}

			// reset 'ready' status of all players
			for (int i = 0; i < 4; i++) {
				ready[i] = false;
			}

			// init game objects
			// init asteroids
			// first, reset them
			for (int i = 0; i < num_asteroids; i++) {
				delete asteroids[i];
			}
			num_asteroids = 0;

			if (selected_stage == stage::fairbanks) {
				asteroids[num_asteroids] = new Asteroid((WIDTH_UNITS - STATUS_BAR_WIDTH) / 2 + STATUS_BAR_WIDTH, HEIGHT_UNITS / 2, 0, 0, r);
				num_asteroids = 1;
			}

			if (selected_stage == stage::utqiagvik) {
				// four slow ice floes, one per quadrant, that bounce around the arena for the whole match
				const int floe_speed = 10000 * 3 / 2;
				const int floe_radius = 10000 * 80;
				for (int i = 0; i < 4; i++) {
					int x = (WIDTH_UNITS - STATUS_BAR_WIDTH) * (i % 2 == 0 ? 1 : 3) / 4 + STATUS_BAR_WIDTH;
					int y = HEIGHT_UNITS * (i < 2 ? 1 : 3) / 4;
					double angle = (rand() * M_PI * 2) / RAND_MAX;
					asteroids[num_asteroids] = new Asteroid(x, y, (int)(floe_speed * cos(angle)), (int)(floe_speed * sin(angle)), r);
					asteroids[num_asteroids]->radius = floe_radius;
					num_asteroids++;
				}
			}

			wind->reset();

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

			const int gun_dirs_x[4] = { 1, -1, 0, 0 };
			const int gun_dirs_y[4] = { 0, 0, 1, -1 };

			if (os == online_status::host) {
				controllers[0].status = client;
			} else if (os == online_status::client) {
				
			}

			for (int i = 0; i < 4; i++) {
				if (ships[i]) {
					delete ships[i];
					ships[i] = NULL;
				}

				if (controllers[i].status == empty) continue;

				int ally1 = -1;
				int ally2 = -1;
				if (selected_team_mode == two_vs_two) {
					if (i == 0) {
						ally1 = 1;
					}
					else if (i == 1) {
						ally1 = 0;
					}
					else if (i == 2) {
						ally1 = 3;
					}
					else if (i == 3) {
						ally1 = 2;
					}
				}

				switch (selections[i]) {
				case ship_type::black:
					ships[i] = new Black(i, ally1, ally2, spawn_locations_x[i], spawn_locations_y[i], r);
					break;
				case ship_type::grizzly:
					ships[i] = new Grizzly(i, ally1, ally2, spawn_locations_x[i], spawn_locations_y[i], r);
					break;
				case ship_type::polar:
					ships[i] = new Polar(i, ally1, ally2, spawn_locations_x[i], spawn_locations_y[i], r);
					break;
				case ship_type::panda:
					ships[i] = new Panda(i, ally1, ally2, spawn_locations_x[i], spawn_locations_y[i], r);
					break;
				case ship_type::kodiak:
					ships[i] = new Kodiak(i, ally1, ally2, spawn_locations_x[i], spawn_locations_y[i], r);
					break;
				default:
					throw;
				}

				ships[i]->gun_dir_x = gun_dirs_x[i];
				ships[i]->gun_dir_y = gun_dirs_y[i];
			}


			for (int i = 0; i < 4; i++) {
				if (!ships[i]) continue;

				ships[i]->lives = stock_count;
			}

			// render background once; this is needed for xp mode
			r->render_texture(bg, WIDTH_UNITS / 2, HEIGHT_UNITS / 2, 0, 1);

			current_state = in_game;
			
		}
		// start of in game state
		else if (current_state == in_game) {
			// poll input
			read_input();

			if (os == online_status::client) {
				goto end_of_update;
			}
			if (os == online_status::local) {
			}

			// ai moves
			for (int i = 0; i < 4; i++) {
				if (controllers[i].status == ai) {
					Input_State a = controllers[i];

					bool lead_target = false;

					switch (selections[i]) {
					case ship_type::black:
						if (!a.lb.state)  a.lb.changed = true;
						a.lb.state = true;
						break;
					case ship_type::grizzly:
						if (!a.rb.state)  a.rb.changed = true;
						a.rb.state = true;
						lead_target = true;
						break;
					case ship_type::polar:
						if (!a.lb.state)  a.lb.changed = true;
						a.lb.state = true;
						break;
					case ship_type::panda:
						if (!a.lb.state)  a.lb.changed = true;
						a.lb.state = true;
						break;
					case ship_type::kodiak:
						if (!a.rb.state)  a.rb.changed = true;
						a.rb.state = true;
						lead_target = true;
						break;
					default:
						throw;
					}

					int goal_x = (WIDTH_UNITS - BARSIZE) / 2 + BARSIZE;
					int goal_y = HEIGHT_UNITS / 2;

					a.l_stick_x = goal_x - ships[i]->x_pos;
					a.l_stick_y = goal_y - ships[i]->y_pos;

					// select target
					int target_ship = -1;
					double min_dist = 9999999999;
					Ship* me = ships[i];
					for (int j = 0; j < 4; j++) {
						if (i == j) continue;

						Ship* them = ships[j];
						if (!them) continue;
						double dist = sqrt(pow(me->x_pos - them->x_pos, 2) + pow(me->y_pos - them->y_pos, 2));
						if (dist < min_dist) {
							min_dist = dist;
							target_ship = j;
						}
					}

					if (target_ship != -1) {
						if (lead_target) {
							double enemy_speed = sqrt(pow(ships[target_ship]->x_vel, 2) + pow(ships[target_ship]->y_vel, 2));
							double bullet_speed = 70000;

							double straight_angle = atan2(ships[target_ship]->y_pos - me->y_pos, ships[target_ship]->x_pos - me->x_pos);

							double beta = atan2(ships[target_ship]->y_vel, ships[target_ship]->x_vel) - straight_angle;
							if (beta < 0) beta += 2 * M_PI;

							double stuff = (enemy_speed * sin(beta)) / bullet_speed;
							if (stuff > 1) stuff = 1;
							if (stuff < -1) stuff = -1;
							double alpha = asin(stuff);

							//std::cout << "beta: " << beta << "\talpha: " << alpha << "\tstuff: " << (enemy_speed * sin(beta)) / bullet_speed << std::endl;

							double aim_angle = straight_angle + alpha;

							a.r_stick_x = 100000 * cos(aim_angle);
							a.r_stick_y = 100000 * sin(aim_angle);
						} else {
							a.r_stick_x = ships[target_ship]->x_pos - me->x_pos;
							a.r_stick_y = ships[target_ship]->y_pos - me->y_pos;
						}
					}
					controllers[i] = a;
				}
			}

			for (int i = 0; i < 4; i++) {
				Input_State is = controllers[i];
				if (is.status == empty) continue;

				Ship* s = ships[i];
				
				s->do_fire_1 = is.rb.state;
				s->do_fire_2 = is.rt.state;
				s->do_fire_3 = is.lb.state;

				s->do_speed_boost = is.l3.state;

				if (is.start.state && is.start.changed && game_start_cooldown < 2 * 60) {
					current_state = pause;
				}
			}

			if (current_state == pause) {
				continue;
			}

			// ----- update -----
			if (game_end_cooldown != game_end_delay) {
				//goto end_of_update;
			}

			// spawn items
			if (do_items) {
				item_spawn_cooldown--;
				if (item_spawn_cooldown == 0) {
					item_spawn_cooldown = (20 * 60) + (rand() % (10 * 60));
					//if (DEBUG_MODE) item_spawn_cooldown = 10 * 60;
					int x_pos = 10000 * (rand() % ((WIDTH_UNITS - STATUS_BAR_WIDTH) / 10000)) + STATUS_BAR_WIDTH;
					int y_pos = 10000 * (rand() % (HEIGHT_UNITS / 10000));
					int type = rand() % NUM_ITEM_TYPES;
					items[num_items] = new Item(x_pos, y_pos, (item_type)type, r);
					num_items++;
				}
			}

			// spawn asteroids
			if (selected_stage == stage::juneau) {
				asteroid_spawn_cooldown--;
				if (asteroid_spawn_cooldown <= 0) {
					asteroid_spawn_cooldown = 10 * 60;

					int x;
					int y;
					{
						long perimeter = 2 * (WIDTH_UNITS - STATUS_BAR_WIDTH) + 2 * HEIGHT_UNITS + 8 * ASTEROID_SPAWN_DIST;
						double perimeter_ratio = (double)(rand()) / RAND_MAX;
						long dist = (int)(perimeter*perimeter_ratio);

						if (dist < (WIDTH_UNITS - STATUS_BAR_WIDTH + 2 * ASTEROID_SPAWN_DIST)) {
							x = dist + STATUS_BAR_WIDTH - ASTEROID_SPAWN_DIST;
							y = -ASTEROID_SPAWN_DIST;
						} else if (dist < (WIDTH_UNITS + HEIGHT_UNITS + 4 * ASTEROID_SPAWN_DIST)) {
							x = WIDTH_UNITS + ASTEROID_SPAWN_DIST;
							y = (dist - (WIDTH_UNITS + 2 * ASTEROID_SPAWN_DIST)) - ASTEROID_SPAWN_DIST + HEIGHT_UNITS;
						} else if (dist < (2 * WIDTH_UNITS + HEIGHT_UNITS + 6 * ASTEROID_SPAWN_DIST)) {
							x = (WIDTH_UNITS + ASTEROID_SPAWN_DIST) - (dist - (WIDTH_UNITS + HEIGHT_UNITS + 4 * ASTEROID_SPAWN_DIST));
							y = HEIGHT_UNITS + ASTEROID_SPAWN_DIST;
						} else {
							x = STATUS_BAR_WIDTH - ASTEROID_SPAWN_DIST;
							y = (HEIGHT_UNITS + ASTEROID_SPAWN_DIST) - (dist - (2 * WIDTH_UNITS + HEIGHT_UNITS + 6 * ASTEROID_SPAWN_DIST));
						}
					}

					double angle = (rand()*M_PI * 2) / RAND_MAX;
					int power = 10000 + (rand() * 50000) / RAND_MAX;

					asteroids[num_asteroids] = new Asteroid(x, y, (int)(power*cos(angle)), (int)(power*sin(angle)), r);
					num_asteroids++;

				}
			}

			// wind gusts
			if (selected_stage == stage::nome && game_start_cooldown == 0 && game_end_cooldown == game_end_delay) {
				wind->update();
			}

			// update asteroids
			for (int i = 0; i < num_asteroids; i++) {
				Asteroid* a = asteroids[i];
				a->update();
				if (selected_stage == stage::utqiagvik) {
					// ice floes bounce off the arena edges instead of drifting away
					if (a->x_pos - a->radius < STATUS_BAR_WIDTH && a->x_vel < 0) a->x_vel = -a->x_vel;
					if (a->x_pos + a->radius > WIDTH_UNITS && a->x_vel > 0) a->x_vel = -a->x_vel;
					if (a->y_pos - a->radius < 0 && a->y_vel < 0) a->y_vel = -a->y_vel;
					if (a->y_pos + a->radius > HEIGHT_UNITS && a->y_vel > 0) a->y_vel = -a->y_vel;
					continue;
				}
				if (a->x_pos < STATUS_BAR_WIDTH - ASTEROID_SPAWN_DIST || a->x_pos > WIDTH_UNITS + ASTEROID_SPAWN_DIST ||
					a->y_pos < -ASTEROID_SPAWN_DIST || a->y_pos > HEIGHT_UNITS + ASTEROID_SPAWN_DIST) {
					delete asteroids[i];
					num_asteroids--;
					asteroids[i] = asteroids[num_asteroids];

					// subtract from the respawn cooldown. this keeps the spawn rate a little more consistent,
					// because if an asteroid goes off-screen quickly a new one comes in faster.
					asteroid_spawn_cooldown -= 5 * 60;
				}
			}

			// update ships
			for (int i = 0; i < 4; i++) {
				if (controllers[i].status == empty) continue;

				Ship* ship = ships[i];

				if (ship->lives == 0) continue;

				if (game_end_cooldown < game_end_delay) continue;

				// update from stick positions
				double left_stick_mag = sqrt(pow(controllers[i].l_stick_x, 2) + pow(controllers[i].l_stick_y, 2));
				if (left_stick_mag > DEAD_ZONE) {
					ship->move_dir_x = controllers[i].l_stick_x;
					ship->move_dir_y = controllers[i].l_stick_y;
					if (game_end_cooldown == game_end_delay) {
						ship->face_dir_x = ship->move_dir_x;
						ship->face_dir_y = ship->move_dir_y;
					}
				} else {
					ship->move_dir_x = 0;
					ship->move_dir_y = 0;
				}

				double right_stick_mag = sqrt(pow(controllers[i].r_stick_x, 2) + pow(controllers[i].r_stick_y, 2));
				//std::cout << right_stick_mag << std::endl;
				if (right_stick_mag > DEAD_ZONE) {
					ship->desired_gun_dir_x = controllers[i].r_stick_x;
					ship->desired_gun_dir_y = controllers[i].r_stick_y;
				}

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

					if (ship->item_times[item_type::small_pwrup] > 60) {
						ship->radius = ship->normal_radius / 2;
						ship->gun_length = ship->normal_gun_length / 2;
					} else if (ship->item_times[item_type::small_pwrup] > 0) {
						if (ship->item_times[item_type::small_pwrup] % 15 < 7) {
							ship->radius = ship->normal_radius;
							ship->gun_length = ship->normal_gun_length;
						} else {
							ship->radius = ship->normal_radius / 2;
							ship->gun_length = ship->normal_gun_length / 2;
						}
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
							accel_mag *= 1.5;
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

						wind->push(ship);

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
						for (int j = i+1; j < 4; j++) {
							if (!ships[j]) continue;
							if (ships[j]->lives == 0) continue;
							double dist = sqrt(pow(ship->x_pos - ships[j]->x_pos, 2) + pow(ship->y_pos - ships[j]->y_pos, 2));
							if (dist == 0) dist = 1;
							if (dist <= (ships[i]->radius + ships[j]->radius)) {
								double total_force = 140000000000000000.0 / pow(dist, 2);
								double x_force = (ship->x_pos - ships[j]->x_pos) * total_force / dist;
								ship->x_vel += (int)(x_force / ship->weight);
								ships[j]->x_vel -= (int)(x_force / ship->weight);
								double y_force = (ship->y_pos - ships[j]->y_pos) * total_force / dist;
								ship->y_vel += (int)(y_force / ship->weight);
								ships[j]->y_vel -= (int)(y_force / ship->weight);
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
							ship->take_knockback(ship->x_pos - a->x_pos, ship->y_pos - a->y_pos, 100, 20, 28, haptics[i]);
						}
					}

					// update position
					ship->x_pos += ship->x_vel;
					ship->y_pos += ship->y_vel;

					// handle death
					int screen_edge_wiggle_room = ship->radius * 1.5;
					if (ship->x_pos < (STATUS_BAR_WIDTH - screen_edge_wiggle_room) || ship->x_pos >(WIDTH_UNITS + screen_edge_wiggle_room)
						|| ship->y_pos < (0 - screen_edge_wiggle_room) || ship->y_pos >(HEIGHT_UNITS + screen_edge_wiggle_room)) {
						if (ship->item_times[bounce] > 0 || ship->invincibility_cooldown > 0) {
							if (ship->x_pos < STATUS_BAR_WIDTH && ship->x_vel < 0) {
								ship->x_vel *= -1;
							} else if (ship->x_pos > WIDTH_UNITS && ship->x_vel > 0) {
								ship->x_vel *= -1;
							} else if (ship->y_pos < 0 && ship->y_vel < 0) {
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

							//if (!DEBUG_MODE) {
								ship->invincibility_cooldown += ship->respawn_invincibility_delay;
							//}

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
				ship->update_projectiles_3(STATUS_BAR_WIDTH, WIDTH_UNITS, 0, HEIGHT_UNITS, ships, asteroids, num_asteroids, haptics);
				ship->update_projectiles_2(STATUS_BAR_WIDTH, WIDTH_UNITS, 0, HEIGHT_UNITS, ships, asteroids, num_asteroids, haptics);

			} // end of update ships

			// Check to see if the game is over
			bool game_over; // declared without initializer so the goto above may legally jump past it
			game_over = false;
			if (selected_team_mode == free_for_all) {
				int number_alive = 0;
				winner = -1;
				for (int i = 0; i < 4; i++) {
					if (!ships[i]) continue;
					if (ships[i]->lives != 0) {
						number_alive++;
						winner = i + 1;
					}
				}
				game_over = (number_alive == 1);
			} else if (selected_team_mode == two_vs_two) {
				int teams_alive = 0;
				if (ships[0]->lives != 0 || ships[1]->lives != 0) {
					teams_alive++;
					winner = 1;
				}
				if (ships[2]->lives != 0 || ships[3]->lives != 0) {
					teams_alive++;
					winner = 3;
				}
				game_over = (teams_alive == 1);
			}

			if (game_start_cooldown > 0) {
				game_start_cooldown--;
			}

			if (game_over) {
				game_end_cooldown--;
			}
			if (game_end_cooldown == 0) {
				current_state = results;

				// remove items
				for (int j = 0; j < num_items; j++) {
					delete items[j];
				}
				num_items = 0;

				// halt all sound effects
				Mix_HaltChannel(-1);
			}

			if (current_state == results) {
				continue;
			}

			end_of_update:


			// get serialized data
			if (os == online_status::host) {
				memset(render_data_buffer, 0, render_data_buffer_size);
				int size = 0;
				// ships
				for (int i = 0; i < 4; i++) {
					if (!ships[i]) continue;
					size = ships[i]->serialize(render_data_buffer, size);
				}

				// asteroids
				size = serialize_int(num_asteroids, render_data_buffer, size);
				for (int i = 0; i < num_asteroids; i++) {
					size = asteroids[i]->serialize(render_data_buffer, size);
				}

				// items
				size = serialize_int(num_items, render_data_buffer, size);
				for (int i = 0; i < num_items; i++) {
					size = items[i]->serialize(render_data_buffer, size);
				}

				// wind
				size = wind->serialize(render_data_buffer, size);

				send_buffer(them, render_data_buffer, render_data_buffer_size);
			}
			else if (os == online_status::client) {
				memset(input_data_buffer, 0, input_data_buffer_size);
				
				controllers[1].serialize(input_data_buffer, 0);

				send_buffer(them, input_data_buffer, input_data_buffer_size);
			}
			else if (os == online_status::local) {
				// just serialize and deserialize for testing
				/*
				int size = 0;
				for (int i = 0; i < 4; i++) {
					if (!ships[i]) continue;
					size = ships[i]->serialize(render_data_buffer, size);
				}

				size = 0;
				for (int i = 0; i < 4; i++) {
					if (!ships[i]) continue;
					size = ships[i]->deserialize(render_data_buffer, size);
				}
				*/
			}

			render_game(game_end_cooldown, game_end_delay, game_start_cooldown,
				countdown_tick, selected_ship, bg,
				ships, items, num_items, asteroids, num_asteroids);
			SDL_RenderPresent(renderer);

		} //end of ingame state

		// start pause state
		else if (current_state == pause) {
			while (SDL_PollEvent(&e)) {
				int controller_index;
				if (read_global_input(&e)) continue;
				switch (e.type) {
				case SDL_CONTROLLERBUTTONDOWN:
					controller_index = lookup_controller(e.cbutton.which);
					if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
						if (LRA[controller_index].a == true && LRA[controller_index].l == true && LRA[controller_index].r == true) {
							winner = -1;
							current_state = results;
						}	
						else {
							current_state = in_game;
						}

						// reset buttons
						for (int i = 0; i < 4; i++) {
							LRA[i].a = false;
							LRA[i].l = false;
							LRA[i].r = false;
						}
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
						LRA[controller_index].a = true;
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_X) {
						xp_mode = !xp_mode;
					}
					else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_Y) {
						if (r->render_normal && !r->render_debug) {
							r->render_debug = true;
						} else if (r->render_normal && r->render_debug) {
							r->render_normal = false;
						} else {
							r->render_normal = true;
							r->render_debug = false;
						}
						render_game(game_end_cooldown, game_end_delay, game_start_cooldown,
							countdown_tick, selected_ship, bg,
							ships, items, num_items, asteroids, num_asteroids);
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
						int min_activation = 17000;
						//std::cout << e.caxis.value << std::endl;
						if (e.caxis.value >= min_activation) {
							LRA[controller_index].r = true;
						} else {
							LRA[controller_index].r = false;
						}
					}
					else if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
						int min_activation = 17000;
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

			// render control instructions
			r->render_text(STATUS_BAR_WIDTH, 0, "L + R + A + START to reset", false, false, true, medium_f, 255, 255);
			r->render_text(STATUS_BAR_WIDTH, r->get_text_height(medium_f), "Y to toggle hitboxes", false, false, true, medium_f, 255, 255);
			r->render_text(STATUS_BAR_WIDTH, 2 * r->get_text_height(medium_f), "X to toggle XP mode", false, false, true, medium_f, 255, 255);


			SDL_RenderPresent(renderer);


		} //end pause state

		// start of options state
		else if (current_state == options) {

		}
		// start of results state
		else if (current_state == results) {
			while (SDL_PollEvent(&e)) {
				if (read_global_input(&e)) continue;
				switch (e.type) {
				case SDL_CONTROLLERBUTTONDOWN:
					if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
						current_state = character_select;
					}
					break;
				}
			}

			// render basic black background
			r->render_solid_bg();

			// render who won plus continue prompt
			{
				char winnerMessage[20];

				if (winner == -1) {
					snprintf(winnerMessage, sizeof(winnerMessage), "No Contest!");
				}
				else if (selected_team_mode == free_for_all) {
					snprintf(winnerMessage, sizeof(winnerMessage), "Player %d wins!", winner);
				}
				else if (selected_team_mode == two_vs_two) {
					if (winner == 1 || winner == 2) {
						snprintf(winnerMessage, sizeof(winnerMessage), "Red Team wins!");
					} else if (winner == 3 || winner == 4) {
						snprintf(winnerMessage, sizeof(winnerMessage), "Blue Team wins!");
					}
				}
				r->render_text(WIDTH_UNITS / 2, HEIGHT_UNITS / 12, winnerMessage, true, false, false, medium_f, 255, 255);

				int start_height = HEIGHT_UNITS / 4;
				if (ships[0] != NULL) {
					render_results(WIDTH_UNITS / 8, start_height, ship_textures[(int)selections[0]][red], ships[0]);
				} 
				if (ships[1] != NULL) {
					color c;
					if (selected_team_mode == two_vs_two) c = magenta;
					else c = blue;
					render_results(3 * WIDTH_UNITS / 8, start_height, ship_textures[(int)selections[1]][c], ships[1]);
				}
				if (ships[2] != NULL) {
					color c;
					if (selected_team_mode == two_vs_two) c = blue;
					else c = yellow;
					render_results(5 * WIDTH_UNITS / 8, start_height, ship_textures[(int)selections[2]][c], ships[2]);
				}
				if (ships[3] != NULL) {
					color c;
					if (selected_team_mode == two_vs_two) c = teal;
					else c = green;
					render_results(7 * WIDTH_UNITS / 8, start_height, ship_textures[(int)selections[3]][c], ships[3]);
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


void log_joystick_devices(const char* when) {
	int n = SDL_NumJoysticks();
	printf("[gc-debug] (%s) SDL sees %d joystick(s)\n", when, n);
	for (int i = 0; i < n; i++) {
		char guid_str[64];
		SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(i), guid_str, sizeof(guid_str));
		const char* name = SDL_JoystickNameForIndex(i);
		printf("[gc-debug]   #%d \"%s\" guid=%s vendor=0x%04X product=0x%04X is_gamecontroller=%s\n",
			i,
			name ? name : "(null)",
			guid_str,
			SDL_JoystickGetDeviceVendor(i),
			SDL_JoystickGetDeviceProduct(i),
			SDL_IsGameController(i) ? "yes" : "no");
	}
}

void log_controller_diagnostics() {
	SDL_version compiled;
	SDL_version linked;
	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);
	printf("[gc-debug] SDL compiled against %d.%d.%d, running %d.%d.%d\n",
		compiled.major, compiled.minor, compiled.patch,
		linked.major, linked.minor, linked.patch);

#ifdef _WIN32
	char dll_path[MAX_PATH] = "(not loaded?)";
	HMODULE sdl_dll = GetModuleHandleA("SDL2.dll");
	if (sdl_dll) GetModuleFileNameA(sdl_dll, dll_path, sizeof(dll_path));
	printf("[gc-debug] SDL2.dll loaded from: %s\n", dll_path);
#endif

	const char* h_hidapi = SDL_GetHint(SDL_HINT_JOYSTICK_HIDAPI);
	const char* h_gamecube = SDL_GetHint(SDL_HINT_JOYSTICK_HIDAPI_GAMECUBE);
	printf("[gc-debug] hints: JOYSTICK_HIDAPI=%s JOYSTICK_HIDAPI_GAMECUBE=%s\n",
		h_hidapi ? h_hidapi : "(unset, defaults on)",
		h_gamecube ? h_gamecube : "(unset)");

	// raw HID view: is the adapter visible to hidapi at all?
	bool adapter_seen = false;
	SDL_hid_device_info* devs = SDL_hid_enumerate(0, 0);
	if (!devs) {
		printf("[gc-debug] SDL_hid_enumerate returned no HID devices at all\n");
	}
	for (SDL_hid_device_info* d = devs; d; d = d->next) {
		printf("[gc-debug] hid device vendor=0x%04X product=0x%04X usage_page=0x%04X usage=0x%04X product_string=\"%ls\" path=%s\n",
			d->vendor_id, d->product_id, d->usage_page, d->usage,
			d->product_string ? d->product_string : L"(null)",
			d->path ? d->path : "(null)");
		if (d->vendor_id == 0x057E && d->product_id == 0x0337) adapter_seen = true;
	}
	SDL_hid_free_enumeration(devs);

	if (adapter_seen) {
		printf("[gc-debug] gamecube adapter (057E:0337) IS visible to hidapi\n");
		printf("[gc-debug]   if no joystick appears below, make sure a controller is plugged into one of the\n");
		printf("[gc-debug]   adapter's ports; only populated ports show up as joysticks\n");
	} else {
		printf("[gc-debug] gamecube adapter (057E:0337) NOT visible to hidapi\n");
		printf("[gc-debug]   - is the adapter's black USB plug connected? (black = data, gray = power/rumble only)\n");
		printf("[gc-debug]   - if it was ever used with dolphin via zadig, a WinUSB driver is bound to it and hides\n");
		printf("[gc-debug]     it from SDL: device manager -> WUP-028 -> uninstall device (check 'delete driver'), replug\n");
	}

	log_joystick_devices("startup");
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
			toggle_mute();
			
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
	case SDL_JOYDEVICEADDED:
	{
		const char* name = SDL_JoystickNameForIndex(e->jdevice.which);
		printf("[gc-debug] SDL_JOYDEVICEADDED device_index=%d name=\"%s\" vendor=0x%04X product=0x%04X is_gamecontroller=%s\n",
			e->jdevice.which,
			name ? name : "(null)",
			SDL_JoystickGetDeviceVendor(e->jdevice.which),
			SDL_JoystickGetDeviceProduct(e->jdevice.which),
			SDL_IsGameController(e->jdevice.which) ? "yes" : "no");
		// not eaten; SDL_CONTROLLERDEVICEADDED arrives separately if it's a recognized game controller
		break;
	}
	case SDL_JOYDEVICEREMOVED:
		printf("[gc-debug] SDL_JOYDEVICEREMOVED instance_id=%d\n", e->jdevice.which);
		break;
	case SDL_CONTROLLERDEVICEREMAPPED:
		printf("[gc-debug] SDL_CONTROLLERDEVICEREMAPPED instance_id=%d\n", e->cdevice.which);
		break;
	case SDL_CONTROLLERDEVICEADDED:
	{
		printf("[gc-debug] SDL_CONTROLLERDEVICEADDED device_index=%d\n", e->cdevice.which);
		SDL_GameController* temp = SDL_GameControllerOpen(e->cdevice.which);
		if (!temp) {
			std::cout << "could not connect controller: " << SDL_GetError() << std::endl;
		}
		printf("[gc-debug] opened \"%s\" vendor=0x%04X product=0x%04X\n",
			temp ? SDL_GameControllerName(temp) : "(open failed)",
			SDL_GameControllerGetVendor(temp),
			SDL_GameControllerGetProduct(temp));

		// official nintendo gamecube usb adapter (WUP-028)
		bool is_gamecube = SDL_GameControllerGetVendor(temp) == 0x057E && SDL_GameControllerGetProduct(temp) == 0x0337;
		printf("[gc-debug] is_gamecube=%s\n", is_gamecube ? "yes" : "no");
		if (is_gamecube) {
			// SDL's built-in gamecube mapping leaves the digital L full-press unbound (joystick
			// button 11), so bind it to left shoulder to give the pad an LB equivalent
			char* mapping = SDL_GameControllerMapping(temp);
			printf("[gc-debug] current mapping: %s\n", mapping ? mapping : "(none)");
			if (mapping && !SDL_strstr(mapping, "leftshoulder:")) {
				std::string extended = mapping;
				if (extended.back() != ',') extended += ',';
				extended += "leftshoulder:b11,";
				int add_result = SDL_GameControllerAddMapping(extended.c_str());
				printf("[gc-debug] added leftshoulder:b11 mapping, result=%d (1=new, 0=updated, -1=error %s)\n",
					add_result, add_result == -1 ? SDL_GetError() : "");
			}
			SDL_free(mapping);
		}

		SDL_Joystick* joy = SDL_GameControllerGetJoystick(temp);
		int instanceID = SDL_JoystickInstanceID(joy);
		// find first empty slot
		bool found_slot = false;
		for (int i = 0; i < 4; i++) {
			if (controller_mappings[i] == -1 && controllers[i].status == empty) {
				found_slot = true;
				controller_mappings[i] = instanceID;
				controllers[i].controller = temp;
				controllers[i].status = human;
				controllers[i].is_gamecube = is_gamecube;

				haptics[i] = SDL_HapticOpen(e->cdevice.which);
				if (SDL_HapticRumbleInit(haptics[i]) != 0) {
					std::cout << "could not init haptic device" << std::endl;
					// the gamecube adapter has no haptic device, but SDL can rumble it directly
					// (requires the adapter's gray power plug to also be connected)
					SDL_GameControllerRumble(temp, 0xC000, 0xC000, 500);
				}
				SDL_HapticRumblePlay(haptics[i], 1.0f, 500);
				if (!haptics[i]) {
					std::cout << SDL_GetError() << std::endl;
				}
				std::cout << "connecting new controller " << instanceID << " in slot " << i << std::endl;
				break;
			}
		}
		if (!found_slot) {
			printf("[gc-debug] no free slot for controller instance_id=%d; all 4 slots occupied (press X in character select to clear AI players)\n", instanceID);
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
		SDL_GameControllerClose(controllers[slot].controller);
		controllers[slot].status = empty;
		controllers[slot].is_gamecube = false;
		SDL_HapticClose(haptics[slot]);
		haptics[slot] = NULL;
		event_eaten = true;
		break;
	}
	}
	return event_eaten;
}

void read_input() {
	// reset 'changed' status
	for (int i = 0; i < 4; i++) {
		//if (controllers[i].status != human) continue;
		if (controllers[i].status == client) continue;

		controllers[i].a.changed = false;
		controllers[i].b.changed = false;
		controllers[i].x.changed = false;
		controllers[i].y.changed = false;

		controllers[i].start.changed = false;
		controllers[i].select.changed = false;
		controllers[i].xbox.changed = false;
		
		controllers[i].d_u.changed = false;
		controllers[i].d_d.changed = false;
		controllers[i].d_l.changed = false;
		controllers[i].d_r.changed = false;
		
		controllers[i].rb.changed = false;
		controllers[i].lb.changed = false;
		
		controllers[i].rt.changed = false;
		controllers[i].lt.changed = false;
		
		controllers[i].r3.changed = false;
		controllers[i].l3.changed = false;
	}

	const double CONTROLLER_MAX_ANGLE = M_PI / 6;
	const int DEAD_ZONE = 5000;

	SDL_Event e;
	int controller_index;
	while (SDL_PollEvent(&e)) {
		if (read_global_input(&e)) continue;
		switch (e.type) {
		case SDL_CONTROLLERBUTTONDOWN:
			controller_index = lookup_controller(e.cbutton.which);
			if (controllers[controller_index].status != human) break;
			if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
				controllers[controller_index].a.state = true;
				controllers[controller_index].a.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
				controllers[controller_index].b.state = true;
				controllers[controller_index].b.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_X) {
				controllers[controller_index].x.state = true;
				controllers[controller_index].x.changed = true;
				// gamecube pads have no stick clicks; X and Y stand in for L3 (speed boost)
				if (controllers[controller_index].is_gamecube) {
					controllers[controller_index].l3.state = true;
					controllers[controller_index].l3.changed = true;
				}
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_Y) {
				controllers[controller_index].y.state = true;
				controllers[controller_index].y.changed = true;
				if (controllers[controller_index].is_gamecube) {
					controllers[controller_index].l3.state = true;
					controllers[controller_index].l3.changed = true;
				}
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
				controllers[controller_index].start.state = true;
				controllers[controller_index].start.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
				controllers[controller_index].select.state = true;
				controllers[controller_index].select.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_GUIDE) {
				controllers[controller_index].xbox.state = true;
				controllers[controller_index].xbox.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
				controllers[controller_index].d_u.state = true;
				controllers[controller_index].d_u.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
				controllers[controller_index].d_d.state = true;
				controllers[controller_index].d_d.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
				controllers[controller_index].d_l.state = true;
				controllers[controller_index].d_l.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
				controllers[controller_index].d_r.state = true;
				controllers[controller_index].d_r.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
				controllers[controller_index].rb.state = true;
				controllers[controller_index].rb.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
				controllers[controller_index].lb.state = true;
				controllers[controller_index].lb.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSTICK) {
				controllers[controller_index].r3.state = true;
				controllers[controller_index].r3.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSTICK) {
				controllers[controller_index].l3.state = true;
				controllers[controller_index].l3.changed = true;
			}
			break;
		case SDL_CONTROLLERBUTTONUP:
			controller_index = lookup_controller(e.cbutton.which);
			if (controllers[controller_index].status != human) break;
			if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
				controllers[controller_index].a.state = false;
				controllers[controller_index].a.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
				controllers[controller_index].b.state = false;
				controllers[controller_index].b.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_X) {
				controllers[controller_index].x.state = false;
				controllers[controller_index].x.changed = true;
				if (controllers[controller_index].is_gamecube) {
					controllers[controller_index].l3.state = false;
					controllers[controller_index].l3.changed = true;
				}
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_Y) {
				controllers[controller_index].y.state = false;
				controllers[controller_index].y.changed = true;
				if (controllers[controller_index].is_gamecube) {
					controllers[controller_index].l3.state = false;
					controllers[controller_index].l3.changed = true;
				}
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
				controllers[controller_index].start.state = false;
				controllers[controller_index].start.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
				controllers[controller_index].select.state = false;
				controllers[controller_index].select.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_GUIDE) {
				controllers[controller_index].xbox.state = false;
				controllers[controller_index].xbox.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
				controllers[controller_index].d_u.state = false;
				controllers[controller_index].d_u.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
				controllers[controller_index].d_d.state = false;
				controllers[controller_index].d_d.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
				controllers[controller_index].d_l.state = false;
				controllers[controller_index].d_l.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
				controllers[controller_index].d_r.state = false;
				controllers[controller_index].d_r.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
				controllers[controller_index].rb.state = false;
				controllers[controller_index].rb.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
				controllers[controller_index].lb.state = false;
				controllers[controller_index].lb.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSTICK) {
				controllers[controller_index].r3.state = false;
				controllers[controller_index].r3.changed = true;
			} else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSTICK) {
				controllers[controller_index].l3.state = false;
				controllers[controller_index].l3.changed = true;
			}
			break;
		case SDL_CONTROLLERAXISMOTION:
			controller_index = lookup_controller(e.cbutton.which);
			if (controllers[controller_index].status != human) break;
			if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
				int min_activation = 20000;
				int min_deactivation = 18000;
				if (e.caxis.value >= min_activation) {
					controllers[controller_index].rt.state = true;
					controllers[controller_index].rt.changed = true;
				} else if (e.caxis.value <= min_deactivation) {
					controllers[controller_index].rt.state = false;
					controllers[controller_index].rt.changed = true;
				}
			} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
				int min_activation = 20000;
				int min_deactivation = 18000;
				if (e.caxis.value >= min_activation) {
					controllers[controller_index].lt.state = true;
					controllers[controller_index].lt.changed = true;
					// gamecube pads have no left shoulder button; the L trigger acts as LB (fire 3)
					if (controllers[controller_index].is_gamecube) {
						controllers[controller_index].lb.state = true;
						controllers[controller_index].lb.changed = true;
					}
				} else if (e.caxis.value <= min_deactivation) {
					controllers[controller_index].lt.state = false;
					controllers[controller_index].lt.changed = true;
					if (controllers[controller_index].is_gamecube) {
						controllers[controller_index].lb.state = false;
						controllers[controller_index].lb.changed = true;
					}
				}
			}
			
			else if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
				double value = (double)e.caxis.value / 32767;

				double circle_x = sin(value * CONTROLLER_MAX_ANGLE) / sin(CONTROLLER_MAX_ANGLE);
				//std::cout << old_gun_mag << std::endl;
				controllers[controller_index].l_stick_x = (int)(10000 * circle_x);
				
			} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
				double value = (double)e.caxis.value / 32768;

				double circle_y = sin(value * CONTROLLER_MAX_ANGLE) / sin(CONTROLLER_MAX_ANGLE);
				//std::cout << old_gun_mag << std::endl;
				controllers[controller_index].l_stick_y = (int)(10000 * circle_y);

			} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
				double value = (double)e.caxis.value / 32767;

				double circle_x = sin(value * CONTROLLER_MAX_ANGLE) / sin(CONTROLLER_MAX_ANGLE);
				//std::cout << circle_x << std::endl;
				controllers[controller_index].r_stick_x = (int)(10000 * circle_x);
				
			} else if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
				double value = (double)e.caxis.value / 32768;

				double circle_y = sin(value * CONTROLLER_MAX_ANGLE) / sin(CONTROLLER_MAX_ANGLE);
				controllers[controller_index].r_stick_y = (int)(10000 * circle_y);
			}
			break;
		}
	}
}


int lookup_controller(int instanceID) {
	for (int i = 0; i < 4; i++) {
		if (controller_mappings[i] == instanceID) {
			return i;
		}
	}
	return -1;
}

void render_character_selector(int x, int y, SDL_Texture* ship_tex, ship_type shipType, SDL_Texture* right_arrow, SDL_Texture* left_arrow, bool ready, player_status ps, bool has_controller) {
	
	int box_w = (WIDTH_UNITS - BARSIZE) / 2;
	int box_h = (HEIGHT_UNITS - BARSIZE) / 2;

	if (ps == empty) {
		r->render_text(x + box_w / 2, y + box_h / 3, "Plug in controller to join", true, false, false, medium_f, 255, 255);
		r->render_text(x + box_w / 2, y + box_h / 2, "or press X to add an AI player", true, false, false, small_f, 255, 255);
		return;
	}

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
	if (shipType == ship_type::black) {
		name = "BLACK";
		wep1 = "RB: Burst Shot";
		wep2 = "RT: Charge Shot";
		wep3 = "LB: Flamethrower";
	} else if (shipType == ship_type::grizzly) {
		name = "GRIZZLY";
		wep1 = "RB: Bullets";
		wep2 = "RT: Missiles";
		wep3 = "LB: Mines";
	} else if (shipType == ship_type::polar) {
		name = "POLAR";
		wep1 = "RB: Shotgun";
		wep2 = "RT: Gravity Missiles";
		wep3 = "LB: Laser";
	} else if (shipType == ship_type::panda) {
		name = "PANDA";
		wep1 = "RB: Charge Shotgun";
		wep2 = "RT: Squiggle";
		wep3 = "LB: Laser";
		} else if (shipType == ship_type::kodiak) {
		name = "KODIAK";
		wep1 = "RB: Sawblades";
		wep2 = "RT: Harpoon";
		wep3 = "LB: Roar";
	}

	if (ps == ai) {
		name = name.append(" (AI)");
	}
	r->render_text(x + box_w / 2, y + 4 * box_h / 10, name, true, false, false, medium_f, 255, 255);
	r->render_text(x + box_w / 3, y + 6 * box_h / 10, wep1, false, false, false, medium_f, 255, 255);
	r->render_text(x + box_w / 3, y + 7 * box_h / 10, wep3, false, false, false, medium_f, 255, 255);
	r->render_text(x + box_w / 3, y + 8 * box_h / 10, wep2, false, false, false, medium_f, 255, 255);

	// explain how to switch this slot between human and ai control
	if (ps == human) {
		r->render_text(x + box_w / 2, y + 9 * box_h / 10, "Press LB to switch to AI", true, false, false, small_f, 255, 255);
	} else if (ps == ai && has_controller) {
		r->render_text(x + box_w / 2, y + 9 * box_h / 10, "Press LB to switch back to human", true, false, false, small_f, 255, 255);
	} else if (ps == ai) {
		r->render_text(x + box_w / 2, y + 9 * box_h / 10, "AI player (no controller)", true, false, false, small_f, 255, 255);
	}
}

void render_results(int x, int y, SDL_Texture * ship_tex, Ship * ship) {

	char killResult[15];
	char suicideResult[15];
	char damageGiven[30];
	char damageTaken[30];
	snprintf(killResult, sizeof(killResult), "Kills: %d", ship->num_kills - ship->num_suicides);
	snprintf(suicideResult, sizeof(suicideResult), "Self-Kills: %d", ship->num_suicides);
	snprintf(damageGiven, sizeof(damageGiven), "Damage Given: %d%% ", ship->damage_done);
	snprintf(damageTaken, sizeof(damageTaken), "Damage Taken: %d%%", ship->damage_taken);

	r->render_texture(ship_tex, x, y, 1, 5);
	r->render_text(x, y + 10000 * 100, killResult, true, false, false, small_f, 255, 255);
	r->render_text(x, y + 10000 * 130, suicideResult, true, false, false, small_f, 255, 255);
	r->render_text(x, y + 10000 * 160, damageGiven, true, false, false, small_f, 255, 255);
	r->render_text(x, y + 10000 * 190, damageTaken, true, false, false, small_f, 255, 255);
}

void render_game(int game_end_cooldown, int game_end_delay, int game_start_cooldown,
		Mix_Chunk* countdown_tick, Mix_Chunk* selected_ship, SDL_Texture* bg,
		Ship* ships[], Item* items[], int num_items, Asteroid* asteroids[], int num_asteroids) {
	// render background
	if (!xp_mode) {
		r->render_texture(bg, WIDTH_UNITS / 2, HEIGHT_UNITS / 2, 0, 1.02);
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

	wind->render();

	// render UI elements
	{
		// render game start countdown
		if (game_start_cooldown > 0) {
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
			if (game_start_cooldown == 4 * 60) Mix_PlayChannel(-1, countdown_tick, 0);
			if (game_start_cooldown == 3 * 60) Mix_PlayChannel(-1, countdown_tick, 0);
			if (game_start_cooldown == 2 * 60) Mix_PlayChannel(-1, countdown_tick, 0);
			if (game_start_cooldown == 1 * 60) Mix_PlayChannel(-1, selected_ship, 0);

			r->render_text((WIDTH_UNITS - STATUS_BAR_WIDTH) / 2 + STATUS_BAR_WIDTH, HEIGHT_UNITS / 2, s, true, true, true, large_f, 255, 255);
		}

		// render game end text
		if (game_end_cooldown < game_end_delay) {
			r->render_text((WIDTH_UNITS - STATUS_BAR_WIDTH) / 2 + STATUS_BAR_WIDTH, HEIGHT_UNITS / 2, " GAME SET ", true, true, true, large_f, 255, 255);
		}

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
					if (ship->ally1 == -1) r->SetRenderDrawColor(0, 0, 160, SDL_ALPHA_OPAQUE);
					else r->SetRenderDrawColor(160, 0, 160, SDL_ALPHA_OPAQUE);
				} else if (ship->id == 2) {
					if (ship->ally1 == -1) r->SetRenderDrawColor(210, 210, 0, SDL_ALPHA_OPAQUE);
					else r->SetRenderDrawColor(0, 0, 160, SDL_ALPHA_OPAQUE);
				} else {
					if (ship->ally1 == -1) r->SetRenderDrawColor(0, 160, 0, SDL_ALPHA_OPAQUE);
					else r->SetRenderDrawColor(0, 160, 160, SDL_ALPHA_OPAQUE);
				}

				r->render_rect(0, 10000 * (120 + box_height*i), (int)(((double)STATUS_BAR_WIDTH * ship->stamina) / ship->stamina_max), 10000 * 30);
			}

			// render percentages
			{
				char str[10];
				snprintf(str, 10, "P%d: %d%%", i + 1, ship->percent);
				int blue_and_green = 255 - (int)(255 * ship->percent / 300);
				if (blue_and_green < 0) {
					blue_and_green = 0;
				}
				r->render_text(0, 10000 * (61 + box_height * i), str, false, false, false, medium_f, blue_and_green, 255);
			}

			// render stock counter
			{
				char playerLives[20];
				snprintf(playerLives, sizeof(playerLives), "Lives: %d", ship->lives);
				r->render_text(0, 10000 * (10 + box_height * i), playerLives, false, false, false, medium_f, 255, 255);
			}
		}
	}
}

SOCKET connect_to_ip(PCSTR ip_address) {
	const char* port = "7534";

	ADDRINFO hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;

	ADDRINFO* address_info;
	int ret_val = getaddrinfo(ip_address, port, &hints, &address_info);
	if (ret_val != 0) {
		printf("failed to get address info\n");
	}
	if (address_info->ai_next != NULL) {
		printf("more than one address info was returned\n");
	}

	SOCKET sock = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
	if (sock == INVALID_SOCKET) {
		printf("problem\n");
	}

	ret_val = connect(sock, address_info->ai_addr, (int)address_info->ai_addrlen);
	if (ret_val == SOCKET_ERROR) {
		printf("socket error, %d\n", WSAGetLastError());
	}

	return sock;
}

SOCKET get_local_socket() {
	const char* port = "7534";

	ADDRINFO hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;

	ADDRINFO* address_info;
	int ret_val = getaddrinfo(NULL, port, &hints, &address_info);
	if (ret_val != 0) {
		printf("could not get address info\n");
	}

	if (address_info->ai_next != NULL) {
		printf("problem\n");
	}

	SOCKET s = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
	if (s == INVALID_SOCKET) {
		printf("socket creation failure\n");
	}

	int bind_result = bind(s, address_info->ai_addr, (int)address_info->ai_addrlen);
	if (bind_result == SOCKET_ERROR) {
		printf("failed to bind\n");
	}

	return s;
}

void send_buffer(SOCKET them, char* buf, int size) {
	int ret_val = send(them, buf, size, 0);
	if (ret_val == 0) {
		printf("failed to send\n");
	}
}
void get_buffer(SOCKET me, char* buf, int size) {
	int amount_read = recv(me, buf, size, 0);
	if (amount_read == SOCKET_ERROR) {
		printf("holy fucking shit\n");
	}
}

void get_input_data_forever(void* ptr) {
	while (1) {
		//memset(input_data_buffer, 0, input_data_buffer_size);
		get_buffer(me, input_data_buffer, input_data_buffer_size);
		int size = 0;
		for (int i = 0; i < 4; i++) {
			if (controllers[i].status != client) continue;
			size = controllers[i].deserialize(input_data_buffer, size);
			//printf("deserializing controller %d\n", i);
		}

		printf("recieved %d bytes\n", size);
	}
}

void get_render_data_forever(void* ptr) {
	while (1) {
		//memset(render_data_buffer, 0, render_data_buffer_size);
		get_buffer(me, render_data_buffer, render_data_buffer_size);
		int size = 0;

		// ships
		for (int i = 0; i < 4; i++) {
			if (!ships[i]) continue;
			size = ships[i]->deserialize(render_data_buffer, size);
		}

		// asteroids
		size = deserialize_int(&num_asteroids, render_data_buffer, size);
		for (int i = 0; i < num_asteroids; i++) {
			if (!asteroids[i]) asteroids[i] = new Asteroid(r);
			size = asteroids[i]->deserialize(render_data_buffer, size);
		}

		// items
		size = deserialize_int(&num_items, render_data_buffer, size);
		for (int i = 0; i < num_items; i++) {
			if (!items[i]) items[i] = new Item(r);
			size = items[i]->deserialize(render_data_buffer, size);
		}

		// wind
		size = wind->deserialize(render_data_buffer, size);

		printf("recieved %d bytes\n", size);
	}
}

ship_type increment_ship_selection(ship_type st, int amount) {
	int new_selection = (int)st + amount;

	while (new_selection < 0) {
		new_selection += (int)ship_type::MAX;
	}

	new_selection %= (int)ship_type::MAX;

	return (ship_type)new_selection;
}

void toggle_mute() {
	muted = !muted;
	if (muted) {
		Mix_HaltMusic();
		Mix_AllocateChannels(0);
	} else {
		Mix_PlayMusic(music, -1);
		// the last 8 channels are reserved for continuous sfx
		Mix_AllocateChannels(32);
	}
}
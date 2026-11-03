#pragma once

enum class harpoon_state {
	ready,
	flying,
	tethered,
	retracting,

	MAX
};

enum class roar_state {
	idle,
	charging,
	blasting,

	MAX
};

// KODIAK: the heavyweight. Slow, heavy, and built to drag you in and blow you away.
//   RB: Sawblades - spinning boomerang blades that ricochet and come back to be caught
//   RT: Harpoon   - a chained spear that impales an enemy and reels them in
//   LB: Roar      - hold to wind up, release a radial shockwave
class Kodiak : public Ship {
private:
	SDL_Texture* sawblade_tex;
	SDL_Texture* bounce_sawblade_tex;
	SDL_Texture* harpoon_tex;
	SDL_Texture* roar_ring_tex;

	Mix_Chunk* sawblade_throw_sfx;
	Mix_Chunk* sawblade_hit_sfx;
	Mix_Chunk* harpoon_sfx;
	Mix_Chunk* roar_sfx;
	Mix_Chunk* roar_charge_sfx;

	int roar_channel;

	const long normal_max_accel = 4600;
	const long charging_max_accel = 2400;

	// --- sawblades (RB)
	class Sawblade* sawblades[10];
	int num_sawblades = 0;
	const int max_sawblades = 3;
	const int sawblade_delay = 28;
	int sawblade_cooldown = 0;
	const int sawblade_throw_speed = 95000;
	const double sawblade_drag = 0.985;
	const int sawblade_outbound_frames = 65;
	const int sawblade_return_accel = 4200;
	const int sawblade_max_speed = 120000;
	const int sawblade_rehit_delay = 20;

	// --- harpoon (RT)
	harpoon_state h_state = harpoon_state::ready;
	int harpoon_x = 0;
	int harpoon_y = 0;
	int harpoon_x_vel = 0;
	int harpoon_y_vel = 0;
	int harpoon_dist_travelled = 0;
	double harpoon_angle = 0; // degrees, for rendering
	int harpoon_target = -1;
	int harpoon_tether_frames = 0;
	bool harpoon_click_used = false;
	const int harpoon_radius = 7 * 10000;
	const int harpoon_speed = 150000;
	const int harpoon_retract_speed = 260000;
	const int harpoon_max_range = 560 * 10000;
	const int harpoon_tether_max = 55;
	const int harpoon_pull = 5200;
	const int harpoon_self_pull = 1600;
	const int harpoon_damage = 4;
	const int harpoon_delay = 75;
	int harpoon_cooldown = 0;

	// --- roar (LB)
	roar_state r_state = roar_state::idle;
	int roar_charge = 0;
	const int roar_max_charge = 45;
	int roar_charge_at_release = 0;
	int roar_radius = 0;
	int roar_max_radius = 0;
	int roar_frames = 0;
	const int roar_min_max_radius = 120 * 10000;
	const int roar_extra_max_radius = 120 * 10000;
	const int roar_expand_per_frame = 13 * 10000;
	const int roar_delay = 50;
	int roar_cooldown = 0;
	bool roar_hit[4];

	class Spark* sparks[1000];
	int num_sparks = 0;

	void release_roar();

	void add_sparks(int x, int y, int count);
	void update_sparks();
	void render_sparks();

	bool is_enemy(Ship* s);

public:
	Kodiak(int identifier, int a1, int a2, int x, int y, Renderer* rend);
	~Kodiak();

	void update();
	void die();

	void fire_1();
	void update_projectiles_1(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]);
	void render_projectiles_1();

	void fire_2();
	void update_projectiles_2(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]);
	void render_projectiles_2();

	void fire_3();
	void update_projectiles_3(int min_x, int max_x, int min_y, int max_y, Ship* ships[], Asteroid* asteroids[], int num_asteroids, SDL_Haptic* haptics[]);
	void render_projectiles_3();

	int serialize(char* buf, int i);
	int deserialize(char*buf, int i);
};

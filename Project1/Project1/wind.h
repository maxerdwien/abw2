#pragma once

// wind gusts for the nome stage. every few seconds a gust blows in a random direction,
// ramps up, holds, and dies down, pushing every ship along with it.
// the host owns the gust timing (update); the current strength and direction are sent
// to online clients (serialize/deserialize), and both sides animate the streaks in render.
class Wind {
private:
	class Renderer* r;

	static const int NUM_STREAKS = 48;
	int streak_x[NUM_STREAKS];
	int streak_y[NUM_STREAKS];

	int cooldown = 0; // frames until the next gust starts
	int timer = 0;    // frames left in the current gust; 0 = calm
	int duration = 0; // total length of the current gust
	int channel = -1; // mixer channel playing the gust sound, -1 if none
	Mix_Chunk* sfx = NULL;

	// play area bounds, in units
	int min_x;
	int max_x;
	int min_y;
	int max_y;

public:
	static const int PEAK_STRENGTH = 4000;
	static const int RAMP_FRAMES = 60;

	int strength = 0; // per-frame velocity push on every ship, in units
	double angle = 0; // direction the wind blows toward, in radians

	Wind(Renderer* rend, int min_x, int max_x, int min_y, int max_y);

	// calm, with the first gust due in a few seconds. call at the start of every match.
	void reset();

	// advance the gust timers by one frame. host only.
	void update();

	// apply this frame's push to a ship's velocity.
	void push(class Ship* ship);

	// draw the streaks (and move them along). does nothing while calm.
	void render();

	int serialize(char* buf, int i);
	int deserialize(char* buf, int i);
};

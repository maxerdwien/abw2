#include <SDL.h>
#include <SDL_mixer.h>
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "serializer.h"
#include "renderer.h"
#include "spaceship.h"
#include "wind.h"

Wind::Wind(Renderer* rend, int min_x_, int max_x_, int min_y_, int max_y_) {
	r = rend;
	min_x = min_x_;
	max_x = max_x_;
	min_y = min_y_;
	max_y = max_y_;

	sfx = Mix_LoadWAV("../Project1/assets/sounds/wind.wav");

	reset();
}

void Wind::reset() {
	strength = 0;
	angle = 0;
	timer = 0;
	duration = 0;
	cooldown = 3 * 60 + rand() % (3 * 60);
	channel = -1;

	for (int i = 0; i < NUM_STREAKS; i++) {
		streak_x[i] = min_x + 10000 * (rand() % ((max_x - min_x) / 10000));
		streak_y[i] = min_y + 10000 * (rand() % ((max_y - min_y) / 10000));
	}
}

void Wind::update() {
	if (timer > 0) {
		timer--;
		int elapsed = duration - timer;
		if (elapsed < RAMP_FRAMES) {
			// ramping up
			strength = PEAK_STRENGTH * elapsed / RAMP_FRAMES;
		} else if (timer < RAMP_FRAMES) {
			// dying down
			strength = PEAK_STRENGTH * timer / RAMP_FRAMES;
			if (channel != -1) {
				Mix_FadeOutChannel(channel, RAMP_FRAMES * 1000 / 60);
				channel = -1;
			}
		} else {
			strength = PEAK_STRENGTH;
		}

		if (timer == 0) {
			strength = 0;
			cooldown = 6 * 60 + rand() % (6 * 60);
		}
	} else {
		cooldown--;
		if (cooldown <= 0) {
			duration = 4 * 60 + rand() % (3 * 60);
			timer = duration;
			angle = (rand() * M_PI * 2) / RAND_MAX;
			channel = Mix_PlayChannel(-1, sfx, 0);
		}
	}
}

void Wind::push(Ship* ship) {
	if (strength <= 0) return;
	ship->x_vel += (int)(strength * cos(angle));
	ship->y_vel += (int)(strength * sin(angle));
}

void Wind::render() {
	if (strength <= 0) return;

	double ratio = (double)strength / PEAK_STRENGTH;
	int speed = 10000 * (int)(6 + 12 * ratio); // units per frame
	int length = 10000 * (int)(20 + 70 * ratio);
	double dx = cos(angle);
	double dy = sin(angle);

	r->SetRenderDrawBlendMode(SDL_BLENDMODE_BLEND);
	r->SetRenderDrawColor(220, 235, 255, (Uint8)(40 + 140 * ratio));

	for (int i = 0; i < NUM_STREAKS; i++) {
		// vary speed and length a little between streaks for some parallax
		int s = speed + speed * (i % 3) / 4;
		int len = length + length * (i % 3) / 4;

		streak_x[i] += (int)(s * dx);
		streak_y[i] += (int)(s * dy);

		// wrap around the play area
		if (streak_x[i] < min_x - len) streak_x[i] += (max_x - min_x) + 2 * len;
		if (streak_x[i] > max_x + len) streak_x[i] -= (max_x - min_x) + 2 * len;
		if (streak_y[i] < min_y - len) streak_y[i] += (max_y - min_y) + 2 * len;
		if (streak_y[i] > max_y + len) streak_y[i] -= (max_y - min_y) + 2 * len;

		r->render_line_w_end(streak_x[i], streak_y[i], streak_x[i] - (int)(len * dx), streak_y[i] - (int)(len * dy));
	}

	r->SetRenderDrawBlendMode(SDL_BLENDMODE_NONE);
}

int Wind::serialize(char* buf, int i) {
	i = serialize_int(strength, buf, i);
	i = serialize_double(angle, buf, i);
	return i;
}

int Wind::deserialize(char* buf, int i) {
	i = deserialize_int(&strength, buf, i);
	i = deserialize_double(&angle, buf, i);
	return i;
}

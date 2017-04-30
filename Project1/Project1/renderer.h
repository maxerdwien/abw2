#pragma once

#include <SDL_ttf.h>
#include <string>

enum font_size {
	small_f,
	medium_f,
	large_f
};

class Renderer {
private:
	SDL_Renderer* renderer;
	int WINDOW_WIDTH;
	int WINDOW_HEIGHT;
	double ratio;
	TTF_Font* caladea;
	TTF_Font* caladea_small;
	TTF_Font* caladea_large;

public:

	bool render_normal;
	bool render_debug;

	SDL_Texture* hitbox_tex;
	SDL_Texture* hurtbox_tex;
	SDL_Texture* invincible_hurtbox_tex;
	SDL_Texture* activation_hitbox_tex;

	Renderer(struct SDL_Renderer* r, int w, int h);

	void render_texture(SDL_Texture* texture, int x, int y, double angle, double scaling);
	void render_texture_abs_size(SDL_Texture* texture, int x, int y, double angle, int radius);
	void render_squiggle_rectangle(SDL_Texture* texture, int x, int y, double angle, int x_size, int y_size);
	void render_texture_edge_spin(SDL_Texture* texture, int x, int y, double angle, double scaling);

	double atan2_degrees(int x_vel, int y_vel);
	int RenderCopyEx(SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect, const double angle, const SDL_Point* center, const SDL_RendererFlip flip);
	int SetRenderDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	SDL_Texture* LoadTexture(const char* file);

	void render_text(int x, int y, const std::string& s, bool center_x, bool center_y, bool highlight, font_size font_size, int green_and_blue, int alpha);

	void render_line(int x_start, int y_start, int x_dir, int y_dir);
	void render_line_w_end(int x_start, int y_start, int x_end, int y_end);
	void render_line_thick(int x_start, int y_start, int x_dir, int y_dir);

	void render_solid_bg();
	void render_background_image(SDL_Texture* texture);

	void render_cross_bars(int width);

	void render_rect(int x, int y, int w, int h);
};
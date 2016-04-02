#pragma once

class Renderer {
private:
	SDL_Renderer* renderer;
	int WINDOW_WIDTH;
	int WINDOW_HEIGHT;
	TTF_Font* caladea36;
	TTF_Font* caladea54;

public:
	Renderer(struct SDL_Renderer* r, int w, int h);

	void render_texture(SDL_Texture* texture, int x, int y, double angle, double scaling);
	double calculate_angle(int x_vel, int y_vel);
	int RenderCopyEx(SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect, const double angle, const SDL_Point* center, const SDL_RendererFlip flip);
	SDL_Texture* LoadTexture(const char* file);

	void render_text(int x, int y, const std::string& s);
	void render_text_centered(int x, int y, const std::string& s);

	void render_line(int x_start, int y_start, int x_dir, int y_dir);
	void render_line_thick(int x_start, int y_start, int x_dir, int y_dir);

	void render_sparks(int x1, int y1, int x2, int y2);
};
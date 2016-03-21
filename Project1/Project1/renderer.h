#pragma once

extern SDL_Renderer* renderer;

void render_texture(SDL_Texture* texture, int x, int y, double angle, double scaling);
double calculate_angle(int x_vel, int y_vel);
int RenderCopyEx(SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect, const double angle, const SDL_Point* center, const SDL_RendererFlip flip);
SDL_Texture* LoadTexture(const char* file);
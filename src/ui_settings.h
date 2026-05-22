#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include <SDL.h>

void handle_settings_event(SDL_Event *e);
void update_settings(float dt);
void draw_settings(SDL_Renderer *r);

#endif

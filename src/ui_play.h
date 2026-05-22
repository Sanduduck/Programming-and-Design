#ifndef UI_PLAY_H
#define UI_PLAY_H

#include <SDL.h>

void handle_play_event(SDL_Event *e);
void update_play(float dt);
void draw_play(SDL_Renderer *r);

#endif

#ifndef UI_RESULT_H
#define UI_RESULT_H

#include <SDL.h>

void handle_result_event(SDL_Event *e);
void update_result(float dt);
void draw_result(SDL_Renderer *r);

#endif

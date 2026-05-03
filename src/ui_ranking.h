#ifndef UI_RANKING_H
#define UI_RANKING_H

#include <SDL.h>

void handle_ranking_event(SDL_Event *e);
void update_ranking(float dt);
void draw_ranking(SDL_Renderer *r);

#endif

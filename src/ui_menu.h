#ifndef UI_MENU_H
#define UI_MENU_H

#include <SDL.h>

void handle_menu_event(SDL_Event *e);
void update_menu(float dt);
void draw_menu(SDL_Renderer *r);

#endif

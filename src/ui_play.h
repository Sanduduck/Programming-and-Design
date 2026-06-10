#ifndef UI_PLAY_H
#define UI_PLAY_H

#include <SDL.h>

void init_battle(void);
void handle_play_event(SDL_Event *e);
void update_play(float dt);
void draw_play(SDL_Renderer *r);
const char *battle_result_message(void);
int battle_was_won(void);

#endif

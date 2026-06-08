#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <SDL.h>
#include "types.h"

// 현재 게임 상태 (다른 모듈에서 참조)
extern GameState current_state;

void init_game(void);
void shutdown_game(void);

void handle_event(SDL_Event *e);
void update_game(float dt);
void draw_game(SDL_Renderer *r);

void change_state(GameState next);

#endif

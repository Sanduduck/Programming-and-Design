#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#define PLAYER_W 50.0f
#define PLAYER_H 50.0f

#include <SDL.h>
#include "types.h"

extern Player player;

void init_player(void);
void update_player(float dt);
void draw_player(SDL_Renderer *r);
void player_jump(void);
void player_damage(void);

#endif

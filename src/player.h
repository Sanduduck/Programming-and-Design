#include <SDL.h>
#include "types.h"

extern Player player;

void init_player(void);
void update_player(float dt);
void draw_player(SDL_Renderer *r);
void player_jump(void);
void player_damage(void);

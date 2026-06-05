#include <SDL.h>
#include "types.h"

extern GameState current_state;

void init_game(void);
void shutdown_game(void);

void handle_event(SDL_Event *e);
void update_game(float dt);
void draw_game(SDL_Renderer *r);

void change_state(GameState next);

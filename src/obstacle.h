#include <SDL.h>
#include "types.h"

#define MAX_OBSTACLES 64

extern Obstacle obstacles[MAX_OBSTACLES];

void init_obstacles(void);
void spawn_obstacle(float x, float y, float vx, float vy, int w, int h, int pattern_id);
void update_obstacles(float dt);
void draw_obstacles(SDL_Renderer *r);

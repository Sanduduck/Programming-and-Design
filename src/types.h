#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

typedef enum {
    STATE_MAIN_MENU,
    STATE_PLAYING,
    STATE_RESULT,
    STATE_RANKING,
    STATE_SETTINGS
} GameState;

typedef struct {
    float x, y;
    float vx, vy;
    int hp;
    int jump_count;
    int max_jumps;
    int invincible_timer;
    bool on_ground;
} Player;

#endif

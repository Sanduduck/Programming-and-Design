#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

typedef enum {
    STATE_MAIN_MENU,
    STATE_PLAYING
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

typedef struct {
    float x, y;
    float vx, vy;
    int width, height;
    bool active;
    int pattern_id;
} Obstacle;

typedef struct {
    int id;
    int phase;
    int obstacle_count;
    bool drop_heal_item;
} Pattern;

#endif

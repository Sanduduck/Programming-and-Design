#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

// 게임 상태
typedef enum {
    STATE_MAIN_MENU,
    STATE_PLAYING,
    STATE_RESULT,
    STATE_RANKING,
    STATE_SETTINGS
} GameState;

// 플레이어
typedef struct {
    float x, y;
    float vx, vy;
    int hp;
    int jump_count;
    int max_jumps;
    int invincible_timer;
    bool on_ground;
} Player;

// 장애물
typedef struct {
    float x, y;
    float vx, vy;
    int width, height;
    bool active;
    bool scored;
    int pattern_id;
} Obstacle;

// 패턴
typedef struct {
    int id;
    int phase;
    int obstacle_count;
    int score_per_obstacle;
    bool drop_heal_item;
} Pattern;

// 설정
typedef struct {
    int bgm_volume;
    int sfx_volume;
    int key_left;
    int key_right;
    int key_jump;
} Settings;

// 랭킹
typedef struct {
    int score;
    char grade[4];
} RankEntry;

#endif

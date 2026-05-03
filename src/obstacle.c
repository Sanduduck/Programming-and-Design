// 장애물 관리

#include "obstacle.h"

Obstacle obstacles[MAX_OBSTACLES];

void init_obstacles(void) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].active = false;
        obstacles[i].scored = false;
    }
}

void spawn_obstacle(float x, float y, float vx, float vy, int w, int h, int pattern_id) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) {
            obstacles[i].x = x;
            obstacles[i].y = y;
            obstacles[i].vx = vx;
            obstacles[i].vy = vy;
            obstacles[i].width = w;
            obstacles[i].height = h;
            obstacles[i].active = true;
            obstacles[i].scored = false;
            obstacles[i].pattern_id = pattern_id;
            return;
        }
    }
}

void update_obstacles(float dt) {
    // TODO: 위치 갱신, 화면 밖이면 비활성화
    (void)dt;
}

void draw_obstacles(SDL_Renderer *r) {
    SDL_SetRenderDrawColor(r, 255, 100, 100, 255);
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;
        SDL_Rect rect = {
            (int)obstacles[i].x, (int)obstacles[i].y,
            obstacles[i].width, obstacles[i].height
        };
        SDL_RenderFillRect(r, &rect);
    }
}

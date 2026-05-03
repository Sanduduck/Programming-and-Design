// 게임 진행 화면: 평지 + 플레이어 (좌우 이동 + 2단 점프)

#include "ui_play.h"
#include "game_state.h"
#include "player.h"

#define WINDOW_W   1280
#define WINDOW_H   720
#define FLOOR_Y    600

void handle_play_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN && e->key.repeat == 0) {
        if (e->key.keysym.sym == SDLK_SPACE ||
            e->key.keysym.sym == SDLK_w ||
            e->key.keysym.sym == SDLK_UP) {
            player_jump();
        }
        if (e->key.keysym.sym == SDLK_m) {
            // 메뉴로 돌아가기 (테스트용)
            change_state(STATE_MAIN_MENU);
        }
    }
}

void update_play(float dt) {
    update_player(dt);
}

void draw_play(SDL_Renderer *r) {
    // 검은 배경
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    SDL_RenderFillRect(r, &bg);

    // 바닥: 하늘색 선 한 줄
    SDL_SetRenderDrawColor(r, 135, 206, 235, 255);
    SDL_RenderDrawLine(r, 0, FLOOR_Y, WINDOW_W, FLOOR_Y);

    // 플레이어
    draw_player(r);
}

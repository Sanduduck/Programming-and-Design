#include "ui_play.h"
#include "game_state.h"
#include "player.h"
#include "pattern.h"
#include "obstacle.h"
#include "collision.h"

#define WINDOW_W 1280
#define WINDOW_H 720
#define BOX_X 240
#define BOX_Y 120
#define BOX_W 800
#define BOX_H 480
#define FLOOR_Y 600
#define SIGNAL_PATTERN 1

// 플레이어 체력을 왼쪽 위에 네 칸짜리 HP 박스로 표시한다.
static void draw_hp_bar(SDL_Renderer *r) {
    for (int i = 0; i < 4; i++) {
        SDL_Rect hp_box = { 30 + i * 36, 30, 26, 26 };

        if (i < player.hp) {
            SDL_SetRenderDrawColor(r, 235, 80, 80, 255);
            SDL_RenderFillRect(r, &hp_box);
        }

        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        SDL_RenderDrawRect(r, &hp_box);
    }
}

// 패턴 전환 또는 안내가 필요한 순간 화면 중앙에 알림 효과를 그린다.
static void draw_phase_notice(SDL_Renderer *r) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 190);
    SDL_Rect panel = { 0, 250, WINDOW_W, 150 };
    SDL_RenderFillRect(r, &panel);

    if (current_pattern_id == SIGNAL_PATTERN) {
        SDL_SetRenderDrawColor(r, 255, 220, 80, 255);
    } else {
        SDL_SetRenderDrawColor(r, 80, 220, 120, 255);
    }

    for (int i = 0; i < 4; i++) {
        SDL_Rect bar = { 440 + i * 105, 315, 70, 22 };
        SDL_RenderFillRect(r, &bar);
    }
}

// 신호 패턴에서 좌우 송신기 모양의 장식 오브젝트를 그린다.
static void draw_station(SDL_Renderer *r, int x, int is_right_side) {
    SDL_SetRenderDrawColor(r, 95, 105, 120, 255);
    SDL_Rect body = { x, BOX_Y + 190, 54, 170 };
    SDL_RenderFillRect(r, &body);

    SDL_SetRenderDrawColor(r, 210, 220, 235, 255);
    SDL_RenderDrawRect(r, &body);
    SDL_RenderDrawLine(r, x + 27, BOX_Y + 155, x + 27, BOX_Y + 190);
    SDL_RenderDrawLine(r, x + 12, BOX_Y + 160, x + 42, BOX_Y + 160);
    SDL_RenderDrawLine(r, x + 8, BOX_Y + 360, x + 46, BOX_Y + 360);

    int antenna_x = x + (is_right_side ? 12 : 42);
    int wave_dir = is_right_side ? -1 : 1;

    SDL_SetRenderDrawColor(r, 255, 220, 80, 255);
    for (int i = 0; i < 3; i++) {
        int radius = 22 + i * 18;
        int top = BOX_Y + 160 - radius / 2;
        int bottom = BOX_Y + 160 + radius / 2;
        SDL_RenderDrawLine(r, antenna_x, BOX_Y + 160,
                           antenna_x + wave_dir * radius, top);
        SDL_RenderDrawLine(r, antenna_x, BOX_Y + 160,
                           antenna_x + wave_dir * radius, bottom);
    }
}

// 플레이 중 키 입력을 처리한다. W/Space는 점프, M/Esc는 메뉴 복귀이다.
void handle_play_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN && e->key.repeat == 0) {
        if (e->key.keysym.scancode == SDL_SCANCODE_W ||
            e->key.keysym.sym == SDLK_SPACE) {
            player_jump();
        }

        if (e->key.keysym.sym == SDLK_m ||
            e->key.keysym.sym == SDLK_ESCAPE) {
            change_state(STATE_MAIN_MENU);
        }
    }
}

// 플레이 화면의 매 프레임 로직이다.
// 플레이어와 패턴을 업데이트하고, 안내 화면이 아닐 때만 충돌을 검사한다.
void update_play(float dt) {
    update_player(dt);
    update_pattern(dt);

    if (!is_phase_notice_active()) {
        check_all_collisions();
    }
}

// 플레이 화면 전체를 그린다: 배경, 플레이 영역, 패턴 오브젝트, 플레이어, HP, 안내 표시.
void draw_play(SDL_Renderer *r) {
    // 전체 배경과 실제 플레이 영역 박스를 먼저 그린다.
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    SDL_RenderFillRect(r, &bg);

    SDL_SetRenderDrawColor(r, 18, 18, 24, 255);
    SDL_Rect box = { BOX_X, BOX_Y, BOX_W, BOX_H };
    SDL_RenderFillRect(r, &box);

    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderDrawRect(r, &box);

    SDL_SetRenderDrawColor(r, 135, 206, 235, 255);
    SDL_RenderDrawLine(r, BOX_X, FLOOR_Y, BOX_X + BOX_W, FLOOR_Y);

    // 특정 패턴일 때는 양쪽 신호 장치를 추가로 표시한다.
    if (current_pattern_id == SIGNAL_PATTERN) {
        draw_station(r, BOX_X - 95, 0);
        draw_station(r, BOX_X + BOX_W + 41, 1);
    }

    // 장애물, 플레이어, HP를 순서대로 그려 최종 플레이 화면을 완성한다.
    draw_obstacles(r);
    draw_player(r);
    draw_hp_bar(r);

    if (is_phase_notice_active()) {
        draw_phase_notice(r);
    }
}

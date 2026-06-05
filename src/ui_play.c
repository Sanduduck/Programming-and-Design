// 게임 진행 화면: 평지 + 플레이어 (좌우 이동 + 2단 점프) + 학년별 패턴
// 패턴 진행 순서/전환은 pattern 모듈이 관리 (pattern_start/update/draw/finished)

#include "ui_play.h"
#include "game_state.h"
#include "player.h"
#include "pattern.h"
#include "settings.h"

// HP 바 (오른쪽 위, 4등분)
#define HP_BAR_X   1020
#define HP_BAR_Y   20
#define HP_BAR_W   240
#define HP_BAR_H   30
#define HP_MAX     4

static bool pattern_started = false;

void handle_play_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN && e->key.repeat == 0) {
        // 설정에서 선택한 조작 방식의 점프 키만 사용
        if (e->key.keysym.scancode == g_settings.key_jump) {
            player_jump();
        }
    }
}

void update_play(float dt) {
    // 플레이 화면 첫 진입 시 패턴 시퀀스를 처음부터 시작
    if (!pattern_started) {
        pattern_start();
        pattern_started = true;
    }
    update_player(dt);
    pattern_update(dt);

    // HP 0(게임 오버) 또는 전체 패턴 통과(졸업 클리어) → 결과 화면
    if (player.hp <= 0 || pattern_finished()) {
        pattern_started = false;
        change_state(STATE_RESULT);
    }
}

static void draw_hp_bar(SDL_Renderer *r) {
    int seg_w = HP_BAR_W / HP_MAX;

    // 채워진 칸 (하늘색) — 왼쪽부터 hp 개수만큼
    SDL_SetRenderDrawColor(r, 135, 206, 235, 255);
    for (int i = 0; i < player.hp && i < HP_MAX; i++) {
        SDL_Rect fill = { HP_BAR_X + i * seg_w, HP_BAR_Y, seg_w, HP_BAR_H };
        SDL_RenderFillRect(r, &fill);
    }

    // 외곽 + 4등분 구분선 (흰색)
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_Rect outer = { HP_BAR_X, HP_BAR_Y, HP_BAR_W, HP_BAR_H };
    SDL_RenderDrawRect(r, &outer);
    for (int i = 1; i < HP_MAX; i++) {
        int gx = HP_BAR_X + i * seg_w;
        SDL_RenderDrawLine(r, gx, HP_BAR_Y, gx, HP_BAR_Y + HP_BAR_H - 1);
    }
}

void draw_play(SDL_Renderer *r) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    SDL_RenderFillRect(r, &bg);

    SDL_SetRenderDrawColor(r, 135, 206, 235, 255);
    SDL_RenderDrawLine(r, 0, FLOOR_Y, WINDOW_W, FLOOR_Y);

    // 현재 진행 중인 패턴 렌더
    pattern_draw(r);
    draw_player(r);
    draw_hp_bar(r);
}

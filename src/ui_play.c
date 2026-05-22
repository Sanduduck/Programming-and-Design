// 게임 진행 화면: 플레이어 (좌우 이동 + 2단 점프) + 페이즈 패턴 자동 순차 진행.
// Phase 1 (1학년) → Phase 2 (2학년) → Phase 3 (3학년) → 졸업(결과).
// 각 페이즈 안에서 enum 순서(0→3)대로 과목 자동 진행. 미구현 과목은 즉시 finished → 건너뜀.

#include "ui_play.h"
#include "game_state.h"
#include "player.h"
#include "phase1.h"
#include "phase2.h"
#include "phase3.h"
#include "settings.h"

#define WINDOW_W   1280
#define WINDOW_H   720
#define FLOOR_Y    600

#define HP_BAR_X   1020
#define HP_BAR_Y   20
#define HP_BAR_W   240
#define HP_BAR_H   30
#define HP_MAX     4

#define PATTERNS_PER_PHASE  4
#define LAST_PHASE          3   // Phase 4 미구현 — 추가 시 4로 변경

static bool pattern_started = false;
static int  current_phase   = 1;
static int  current_pattern = 0;

static void start_current(void) {
    if      (current_phase == 1) phase1_start((Phase1PatternId)current_pattern);
    else if (current_phase == 2) phase2_start((Phase2PatternId)current_pattern);
    else if (current_phase == 3) phase3_start((Phase3PatternId)current_pattern);
}

static void advance_pattern(void) {
    current_pattern++;
    if (current_pattern >= PATTERNS_PER_PHASE) {
        current_pattern = 0;
        current_phase++;
    }
    if (current_phase > LAST_PHASE) {
        pattern_started = false;
        change_state(STATE_RESULT);
        return;
    }
    start_current();
}

void handle_play_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN && e->key.repeat == 0) {
        if (e->key.keysym.scancode == g_settings.key_jump ||
            e->key.keysym.scancode == SDL_SCANCODE_W) {
            player_jump();
        }
        if (e->key.keysym.scancode == SDL_SCANCODE_Q) {
            current_phase = 2;
            current_pattern = 0;
            start_current();
            pattern_started = true;
        }
        if (e->key.keysym.scancode == SDL_SCANCODE_E) {
            current_phase = 3;
            current_pattern = 0;
            start_current();
            pattern_started = true;
        }
        if (e->key.keysym.scancode == SDL_SCANCODE_P && pattern_started) {
            advance_pattern();
        }
    }
}

void update_play(float dt) {
    if (!pattern_started) {
        current_phase   = 1;
        current_pattern = 0;
        start_current();
        pattern_started = true;
    }
    update_player(dt);

    bool finished = false;
    if      (current_phase == 1) { phase1_update(dt); finished = phase1_pattern_finished(); }
    else if (current_phase == 2) { phase2_update(dt); finished = phase2_pattern_finished(); }
    else if (current_phase == 3) { phase3_update(dt); finished = phase3_pattern_finished(); }

    if (finished) advance_pattern();

    if (player.hp <= 0) {
        pattern_started = false;
        change_state(STATE_RESULT);
    }
}

static void draw_hp_bar(SDL_Renderer *r) {
    int seg_w = HP_BAR_W / HP_MAX;

    SDL_SetRenderDrawColor(r, 135, 206, 235, 255);
    for (int i = 0; i < player.hp && i < HP_MAX; i++) {
        SDL_Rect fill = { HP_BAR_X + i * seg_w, HP_BAR_Y, seg_w, HP_BAR_H };
        SDL_RenderFillRect(r, &fill);
    }

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

    if      (current_phase == 1) phase1_draw(r);
    else if (current_phase == 2) phase2_draw(r);
    else if (current_phase == 3) phase3_draw(r);

    draw_player(r);
    draw_hp_bar(r);
}

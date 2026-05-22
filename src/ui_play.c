// 게임 진행 화면: 평지 + 플레이어 (좌우 이동 + 2단 점프) + 학년별 패턴
// 학년 전환: Q=1학년, W=2학년, E=3학년
// 과목 선택: 활성 학년에 따라 숫자 키 매핑이 달라짐

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

// HP 바 (오른쪽 위, 4등분)
#define HP_BAR_X   1020
#define HP_BAR_Y   20
#define HP_BAR_W   240
#define HP_BAR_H   30
#define HP_MAX     4

// 현재 활성 학년 — 숫자 키 매핑이 이 값에 따라 분기됨
typedef enum {
    ACTIVE_PHASE_1 = 0,
    ACTIVE_PHASE_2,
    ACTIVE_PHASE_3
} ActivePhase;

static ActivePhase active_phase = ACTIVE_PHASE_3;
static bool pattern_started = false;

void handle_play_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN && e->key.repeat == 0) {
        // 설정에서 선택한 조작 방식의 점프 키만 사용
        if (e->key.keysym.scancode == g_settings.key_jump) {
            player_jump();
        }

        // 학년 전환 핫키 (개발용) — 누른 순간 해당 학년의 기본 패턴으로 진입
        if (e->key.keysym.sym == SDLK_q) {
            active_phase = ACTIVE_PHASE_1;
            phase1_start(PHASE1_PROGLANG);
        }
        if (e->key.keysym.sym == SDLK_w) {
            active_phase = ACTIVE_PHASE_2;
            phase2_start(PHASE2_DATACOMM);
        }
        if (e->key.keysym.sym == SDLK_e) {
            active_phase = ACTIVE_PHASE_3;
            phase3_start(PHASE3_BLOCKCHAIN);
        }

        // 과목 선택 — 현재 학년에 따라 숫자 키 의미가 달라짐
        if (active_phase == ACTIVE_PHASE_1) {
            // 1학년: 1=프로그래밍언어
            if (e->key.keysym.sym == SDLK_1) { phase1_start(PHASE1_PROGLANG); }
        } else if (active_phase == ACTIVE_PHASE_2) {
            // 2학년: 1=데이터통신
            if (e->key.keysym.sym == SDLK_1) { phase2_start(PHASE2_DATACOMM); }
        } else if (active_phase == ACTIVE_PHASE_3) {
            // 3학년: 1=블록체인 / 2=OS / 3=ML / 4=웹서버
            if (e->key.keysym.sym == SDLK_1) { phase3_start(PHASE3_BLOCKCHAIN); }
            if (e->key.keysym.sym == SDLK_2) { phase3_start(PHASE3_OS); }
            if (e->key.keysym.sym == SDLK_3) { phase3_start(PHASE3_ML); }
            if (e->key.keysym.sym == SDLK_4) { phase3_start(PHASE3_WEBSERVER); }
        }
    }
}

void update_play(float dt) {
    // 플레이 화면 첫 진입 시 기본 패턴 (3학년 블록체인) 자동 시작
    if (!pattern_started) {
        active_phase = ACTIVE_PHASE_3;
        phase3_start(PHASE3_BLOCKCHAIN);
        pattern_started = true;
    }
    update_player(dt);

    // 활성 학년의 패턴만 업데이트
    switch (active_phase) {
        case ACTIVE_PHASE_1: phase1_update(dt); break;
        case ACTIVE_PHASE_2: phase2_update(dt); break;
        case ACTIVE_PHASE_3: phase3_update(dt); break;
    }

    // HP 0 → 결과 화면 (게임 오버)
    if (player.hp <= 0) {
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

    // 활성 학년의 패턴만 렌더
    switch (active_phase) {
        case ACTIVE_PHASE_1: phase1_draw(r); break;
        case ACTIVE_PHASE_2: phase2_draw(r); break;
        case ACTIVE_PHASE_3: phase3_draw(r); break;
    }
    draw_player(r);
    draw_hp_bar(r);
}

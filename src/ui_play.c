// 게임 진행 화면: 평지 + 플레이어 (좌우 이동 + 2단 점프) + 학년별 패턴
// 패턴 진행 순서/전환은 pattern 모듈이 관리 (pattern_start/update/draw/finished)

#include "ui_play.h"
#include "game_state.h"
#include "player.h"
#include "pattern.h"
#include "settings.h"
#include "score.h"
#include <SDL_ttf.h>

// HP 바 (오른쪽 위, 4등분)
#define HP_BAR_X   1020
#define HP_BAR_Y   20
#define HP_BAR_W   240
#define HP_BAR_H   30
#define HP_MAX     4

#define QUIT_BOX_X 390
#define QUIT_BOX_Y 235
#define QUIT_BOX_W 500
#define QUIT_BOX_H 250
#define BUTTON_W   150
#define BUTTON_H   55
#define YES_X      455
#define NO_X       675
#define BUTTON_Y   390

static bool pattern_started = false;
static bool quit_confirm = false;
static float score_timer = 0.0f;
static TTF_Font *hud_font = NULL;
static TTF_Font *dialog_font = NULL;

static void load_fonts(void) {
    if (!hud_font) {
        hud_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 28);
    }
    if (!dialog_font) {
        dialog_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 32);
    }
}

static void draw_text_centered(SDL_Renderer *r, TTF_Font *font,
                               const char *text, int x, int y, int w, int h) {
    if (!font) return;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, white);
    if (!surface) return;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(r, surface);
    if (texture) {
        SDL_Rect dst = {
            x + (w - surface->w) / 2,
            y + (h - surface->h) / 2,
            surface->w,
            surface->h
        };
        SDL_RenderCopy(r, texture, NULL, &dst);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

static bool point_in_rect(int x, int y, int rx, int ry, int rw, int rh) {
    return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}

static void confirm_quit(void) {
    quit_confirm = false;
    pattern_started = false;
    change_state(STATE_RESULT);
}

void handle_play_event(SDL_Event *e) {
    if (quit_confirm) {
        if (e->type == SDL_KEYDOWN && e->key.repeat == 0) {
            if (e->key.keysym.sym == SDLK_y ||
                e->key.keysym.sym == SDLK_RETURN) {
                confirm_quit();
            } else if (e->key.keysym.sym == SDLK_n ||
                       e->key.keysym.sym == SDLK_ESCAPE) {
                quit_confirm = false;
            }
        } else if (e->type == SDL_MOUSEBUTTONDOWN &&
                   e->button.button == SDL_BUTTON_LEFT) {
            int mx = e->button.x;
            int my = e->button.y;
            if (point_in_rect(mx, my, YES_X, BUTTON_Y, BUTTON_W, BUTTON_H)) {
                confirm_quit();
            } else if (point_in_rect(
                           mx, my, NO_X, BUTTON_Y, BUTTON_W, BUTTON_H)) {
                quit_confirm = false;
            }
        }
        return;
    }

    if (e->type == SDL_KEYDOWN && e->key.repeat == 0) {
        if (e->key.keysym.sym == SDLK_ESCAPE) {
            quit_confirm = true;
            return;
        }
        // 설정에서 선택한 조작 방식의 점프 키만 사용
        if (e->key.keysym.scancode == g_settings.key_jump) {
            player_jump();
        }
    }
}

void update_play(float dt) {
    if (quit_confirm) return;

    // 플레이 화면 첫 진입 시 패턴 시퀀스를 처음부터 시작
    if (!pattern_started) {
        pattern_start();
        pattern_started = true;
        score_timer = 0.0f;
    }

    // 생존 시간에 따라 내부 점수를 계속 누적한다. (초당 10점)
    score_timer += dt;
    while (score_timer >= 0.1f) {
        add_score(1);
        score_timer -= 0.1f;
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

static void draw_grade(SDL_Renderer *r) {
    char grade[4];
    calc_grade(total_score, player.hp, grade);
    draw_text_centered(r, hud_font, grade, 900, 15, 100, 45);
}

static void draw_quit_confirm(SDL_Renderer *r) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
    SDL_Rect shade = {0, 0, WINDOW_W, WINDOW_H};
    SDL_RenderFillRect(r, &shade);

    SDL_SetRenderDrawColor(r, 35, 35, 45, 255);
    SDL_Rect dialog = {QUIT_BOX_X, QUIT_BOX_Y, QUIT_BOX_W, QUIT_BOX_H};
    SDL_RenderFillRect(r, &dialog);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderDrawRect(r, &dialog);

    draw_text_centered(
        r, dialog_font, "메인 화면으로 돌아갈까요?",
        QUIT_BOX_X, QUIT_BOX_Y + 45, QUIT_BOX_W, 55);
    draw_text_centered(
        r, hud_font, "현재 점수는 결과 화면에 표시됩니다.",
        QUIT_BOX_X, QUIT_BOX_Y + 110, QUIT_BOX_W, 40);

    SDL_SetRenderDrawColor(r, 0, 150, 110, 255);
    SDL_Rect yes = {YES_X, BUTTON_Y, BUTTON_W, BUTTON_H};
    SDL_RenderFillRect(r, &yes);
    SDL_SetRenderDrawColor(r, 120, 70, 70, 255);
    SDL_Rect no = {NO_X, BUTTON_Y, BUTTON_W, BUTTON_H};
    SDL_RenderFillRect(r, &no);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderDrawRect(r, &yes);
    SDL_RenderDrawRect(r, &no);
    draw_text_centered(r, hud_font, "Yes (Y)", YES_X, BUTTON_Y, BUTTON_W, BUTTON_H);
    draw_text_centered(r, hud_font, "No (N)", NO_X, BUTTON_Y, BUTTON_W, BUTTON_H);
}

void draw_play(SDL_Renderer *r) {
    load_fonts();

    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    SDL_RenderFillRect(r, &bg);

    SDL_SetRenderDrawColor(r, 135, 206, 235, 255);
    SDL_RenderDrawLine(r, 0, FLOOR_Y, WINDOW_W, FLOOR_Y);

    // 현재 진행 중인 패턴 렌더
    pattern_draw(r);
    draw_player(r);
    draw_hp_bar(r);
    draw_grade(r);

    if (quit_confirm) draw_quit_confirm(r);
}

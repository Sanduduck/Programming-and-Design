// 설정 화면: 마스터/BGM/효과음 볼륨 슬라이더 + 조작키 설정 (WASD / 방향키)

#include <SDL_ttf.h>
#include <stdio.h>
#include "ui_settings.h"
#include "game_state.h"
#include "settings.h"

// 슬라이더 공통 좌표
#define SLIDER_X        500
#define SLIDER_W        500
#define SLIDER_H        24
#define SLIDER_LABEL_X  220
#define SLIDER_VALUE_X  1030
#define SLIDER_HIT_PAD  16

// 슬라이더 라벨 y (트랙은 +10)
#define MASTER_Y 140
#define BGM_Y    210
#define SFX_Y    280

// 조작키 박스
#define BOX_TOP_Y    420
#define BOX_H        200
#define BOX_W        440
#define BOX_LEFT_X   160     // WASD
#define BOX_RIGHT_X  680     // 방향키

static TTF_Font *font_l = NULL;   // 큰 글자 (타이틀/박스 제목)
static TTF_Font *font_m = NULL;   // 중간 글자 (본문)
static int dragging_slider = -1;  // -1 없음 / 0 마스터 / 1 BGM / 2 효과음

static void load_fonts(void) {
    if (font_l) return;
    font_l = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 32);
    font_m = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 22);
}

static void draw_text(SDL_Renderer *r, TTF_Font *f, const char *txt,
                      int x, int y, SDL_Color c) {
    SDL_Surface *surf = TTF_RenderUTF8_Blended(f, txt, c);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst = { x, y, surf->w, surf->h };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

static void draw_text_centered(SDL_Renderer *r, TTF_Font *f, const char *txt,
                               int box_x, int box_y, int box_w, int box_h,
                               SDL_Color c) {
    SDL_Surface *surf = TTF_RenderUTF8_Blended(f, txt, c);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst = {
        box_x + (box_w - surf->w) / 2,
        box_y + (box_h - surf->h) / 2,
        surf->w, surf->h
    };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

static int slider_track_y(int idx) {
    if (idx == 0) return MASTER_Y + 10;
    if (idx == 1) return BGM_Y + 10;
    return SFX_Y + 10;
}

static int hit_slider(int mx, int my, int idx) {
    int sy = slider_track_y(idx);
    return (mx >= SLIDER_X - SLIDER_HIT_PAD &&
            mx <= SLIDER_X + SLIDER_W + SLIDER_HIT_PAD &&
            my >= sy - SLIDER_HIT_PAD &&
            my <= sy + SLIDER_H + SLIDER_HIT_PAD);
}

static int mx_to_volume(int mx) {
    int v = (mx - SLIDER_X) * 100 / SLIDER_W;
    if (v < 0)   v = 0;
    if (v > 100) v = 100;
    return v;
}

static int *volume_ptr(int idx) {
    if (idx == 0) return &g_settings.master_volume;
    if (idx == 1) return &g_settings.bgm_volume;
    return &g_settings.sfx_volume;
}

static int hit_box(int mx, int my, int box_x) {
    return (mx >= box_x && mx < box_x + BOX_W &&
            my >= BOX_TOP_Y && my < BOX_TOP_Y + BOX_H);
}

void handle_settings_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_ESCAPE:
                change_state(STATE_MAIN_MENU);
                return;
            case SDLK_1:
            case SDLK_KP_1:
                settings_set_scheme(CONTROL_WASD);
                return;
            case SDLK_2:
            case SDLK_KP_2:
                settings_set_scheme(CONTROL_ARROWS);
                return;
        }
    } else if (e->type == SDL_MOUSEBUTTONDOWN &&
               e->button.button == SDL_BUTTON_LEFT) {
        int mx = e->button.x, my = e->button.y;
        for (int i = 0; i < 3; i++) {
            if (hit_slider(mx, my, i)) {
                dragging_slider = i;
                *volume_ptr(i) = mx_to_volume(mx);
                return;
            }
        }
        if (hit_box(mx, my, BOX_LEFT_X)) {
            settings_set_scheme(CONTROL_WASD);
        } else if (hit_box(mx, my, BOX_RIGHT_X)) {
            settings_set_scheme(CONTROL_ARROWS);
        }
    } else if (e->type == SDL_MOUSEBUTTONUP &&
               e->button.button == SDL_BUTTON_LEFT) {
        dragging_slider = -1;
    } else if (e->type == SDL_MOUSEMOTION && dragging_slider >= 0) {
        *volume_ptr(dragging_slider) = mx_to_volume(e->motion.x);
    }
}

void update_settings(float dt) {
    (void)dt;
}

static void draw_slider(SDL_Renderer *r, int idx,
                        const char *label, int volume) {
    int label_y = (idx == 0) ? MASTER_Y : (idx == 1 ? BGM_Y : SFX_Y);
    int sy = slider_track_y(idx);
    SDL_Color white = { 255, 255, 255, 255 };

    draw_text(r, font_m, label, SLIDER_LABEL_X, label_y, white);

    // 트랙 배경 + 테두리
    SDL_Rect track = { SLIDER_X, sy, SLIDER_W, SLIDER_H };
    SDL_SetRenderDrawColor(r, 60, 60, 60, 255);
    SDL_RenderFillRect(r, &track);
    SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
    SDL_RenderDrawRect(r, &track);

    // 채워진 부분 (시안)
    int fill_w = SLIDER_W * volume / 100;
    SDL_Rect fill = { SLIDER_X, sy, fill_w, SLIDER_H };
    SDL_SetRenderDrawColor(r, 0, 200, 220, 255);
    SDL_RenderFillRect(r, &fill);

    // 핸들
    SDL_Rect handle = { SLIDER_X + fill_w - 4, sy - 6, 8, SLIDER_H + 12 };
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderFillRect(r, &handle);

    // 값
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", volume);
    draw_text(r, font_m, buf, SLIDER_VALUE_X, label_y, white);
}

static void draw_scheme_box(SDL_Renderer *r, int box_x, int selected,
                            const char *title, const char *l_str,
                            const char *r_str, const char *j_str) {
    SDL_Rect box = { box_x, BOX_TOP_Y, BOX_W, BOX_H };

    // 선택된 박스: 어두운 시안 채움 + 굵은 흰 테두리 (이중선)
    if (selected) {
        SDL_SetRenderDrawColor(r, 0, 110, 130, 255);
        SDL_RenderFillRect(r, &box);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        SDL_RenderDrawRect(r, &box);
        SDL_Rect inner = { box_x + 2, BOX_TOP_Y + 2, BOX_W - 4, BOX_H - 4 };
        SDL_RenderDrawRect(r, &inner);
    } else {
        SDL_SetRenderDrawColor(r, 120, 120, 120, 255);
        SDL_RenderDrawRect(r, &box);
    }

    SDL_Color white = { 255, 255, 255, 255 };
    draw_text_centered(r, font_l, title,
                       box_x, BOX_TOP_Y + 12, BOX_W, 40, white);
    draw_text(r, font_m, l_str, box_x + 70, BOX_TOP_Y + 80,  white);
    draw_text(r, font_m, r_str, box_x + 70, BOX_TOP_Y + 115, white);
    draw_text(r, font_m, j_str, box_x + 70, BOX_TOP_Y + 150, white);
}

void draw_settings(SDL_Renderer *r) {
    load_fonts();

    // 검은 배경
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    SDL_RenderFillRect(r, &bg);

    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color cyan  = { 0, 230, 230, 255 };

    // 타이틀
    draw_text_centered(r, font_l, "설정", 0, 50, WINDOW_W, 50, cyan);

    // 볼륨 슬라이더 3개
    draw_slider(r, 0, "마스터 볼륨", g_settings.master_volume);
    draw_slider(r, 1, "배경음악",   g_settings.bgm_volume);
    draw_slider(r, 2, "효과음",     g_settings.sfx_volume);

    // 조작키 안내
    draw_text_centered(r, font_m,
                       "조작키 설정   ( 1: WASD   2: 방향키 )",
                       0, 370, WINDOW_W, 30, white);

    // 좌/우 박스 — 선택된 쪽 강조
    int wasd = (g_settings.control_scheme == CONTROL_WASD);
    draw_scheme_box(r, BOX_LEFT_X,  wasd,
                    "1. WASD",
                    "왼쪽 :  A",
                    "오른쪽:  D",
                    "점프 :  W");
    draw_scheme_box(r, BOX_RIGHT_X, !wasd,
                    "2. 방향키",
                    "왼쪽 :  ←",
                    "오른쪽:  →",
                    "점프 :  ↑");

    // 하단 안내
    draw_text_centered(r, font_m,
                       "ESC : 메뉴 복귀   (마우스 클릭으로도 선택 가능)",
                       0, 660, WINDOW_W, 30, white);
}

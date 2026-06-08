// 결과 화면: 누적 점수와 학점 표시

#include "ui_result.h"
#include "game_state.h"
#include "score.h"
#include "player.h"
#include <SDL_ttf.h>
#include <stdio.h>

static TTF_Font *title_font = NULL;
static TTF_Font *score_font = NULL;
static TTF_Font *guide_font = NULL;

static void load_fonts(void) {
    if (!title_font) {
        title_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 54);
    }
    if (!score_font) {
        score_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 42);
    }
    if (!guide_font) {
        guide_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 26);
    }
}

static void draw_text_centered(SDL_Renderer *r, TTF_Font *font,
                               const char *text, int y, int height,
                               SDL_Color color) {
    if (!font) return;
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) return;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(r, surface);
    if (texture) {
        SDL_Rect dst = {
            (WINDOW_W - surface->w) / 2,
            y + (height - surface->h) / 2,
            surface->w,
            surface->h
        };
        SDL_RenderCopy(r, texture, NULL, &dst);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void handle_result_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_RETURN ||
            e->key.keysym.sym == SDLK_ESCAPE) {
            change_state(STATE_MAIN_MENU);
        }
    } else if (e->type == SDL_MOUSEBUTTONDOWN &&
               e->button.button == SDL_BUTTON_LEFT) {
        change_state(STATE_MAIN_MENU);
    }
}

void update_result(float dt) {
    (void)dt;
}

void draw_result(SDL_Renderer *r) {
    load_fonts();

    // 검은 배경
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    SDL_RenderFillRect(r, &bg);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color cyan = {0, 220, 230, 255};
    char score_text[64];
    char grade_text[32];
    char grade[4];

    calc_grade(total_score, player.hp, grade);
    snprintf(score_text, sizeof(score_text), "점수: %d", total_score);
    snprintf(grade_text, sizeof(grade_text), "학점: %s", grade);

    draw_text_centered(r, title_font, "게임 결과", 110, 80, cyan);
    draw_text_centered(r, score_font, score_text, 260, 70, white);
    draw_text_centered(r, score_font, grade_text, 350, 70, white);
    draw_text_centered(
        r, guide_font,
        "Enter 또는 ESC를 누르면 메인 화면으로 돌아갑니다.",
        560, 50, white);
}

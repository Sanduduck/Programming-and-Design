#include <SDL_ttf.h>
#include <stdio.h>

#include "game_state.h"
#include "player.h"
#include "score.h"
#include "ui_play.h"
#include "ui_result.h"

static TTF_Font *title_font;
static TTF_Font *body_font;

static void text_center(SDL_Renderer *r, TTF_Font *font, const char *text, int y, SDL_Color color) {
    SDL_Surface *surface = font ? TTF_RenderUTF8_Blended(font, text, color) : NULL;
    if (surface) {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(r, surface);
        SDL_Rect dst = { (1280 - surface->w) / 2, y, surface->w, surface->h };
        if (texture) SDL_RenderCopy(r, texture, NULL, &dst);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
}

void handle_result_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_RETURN || e->key.keysym.sym == SDLK_r)
            change_state(STATE_PLAYING);
        else if (e->key.keysym.sym == SDLK_m || e->key.keysym.sym == SDLK_ESCAPE)
            change_state(STATE_MAIN_MENU);
    }
}

void update_result(float dt) {
    (void)dt;
}

void draw_result(SDL_Renderer *r) {
    SDL_Rect bg = { 0, 0, 1280, 720 };
    SDL_Rect card = { 360, 125, 560, 450 };
    SDL_Color white = { 244, 247, 255, 255 };
    SDL_Color accent = battle_was_won() ?
        (SDL_Color){ 73, 222, 174, 255 } : (SDL_Color){ 240, 100, 121, 255 };
    char grade[4];
    char line[80];

    if (!title_font) title_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 46);
    if (!body_font) body_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 26);
    calc_grade(total_score, player.hp, grade);

    SDL_SetRenderDrawColor(r, 11, 18, 31, 255);
    SDL_RenderFillRect(r, &bg);
    SDL_SetRenderDrawColor(r, 24, 39, 60, 255);
    SDL_RenderFillRect(r, &card);
    SDL_SetRenderDrawColor(r, accent.r, accent.g, accent.b, 255);
    SDL_RenderDrawRect(r, &card);

    text_center(r, title_font, battle_result_message(), 175, accent);
    snprintf(line, sizeof(line), "프로젝트 점수: %d", total_score);
    text_center(r, body_font, line, 285, white);
    snprintf(line, sizeof(line), "남은 체력: %d", player.hp);
    text_center(r, body_font, line, 335, white);
    snprintf(line, sizeof(line), "최종 학점: %s", grade);
    text_center(r, title_font, line, 400, accent);
    text_center(r, body_font, "Enter/R: 재도전     M/ESC: 메인 메뉴", 515, white);
}

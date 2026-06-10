#include <SDL_ttf.h>

#include "game_state.h"
#include "ui_ranking.h"

static TTF_Font *title_font;
static TTF_Font *body_font;

static void text(SDL_Renderer *r, TTF_Font *font, const char *value, int x, int y, SDL_Color color) {
    SDL_Surface *surface = font ? TTF_RenderUTF8_Blended(font, value, color) : NULL;
    if (surface) {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(r, surface);
        SDL_Rect dst = { x, y, surface->w, surface->h };
        if (texture) SDL_RenderCopy(r, texture, NULL, &dst);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
}

void handle_ranking_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN || e->type == SDL_MOUSEBUTTONDOWN)
        change_state(STATE_MAIN_MENU);
}

void update_ranking(float dt) {
    (void)dt;
}

void draw_ranking(SDL_Renderer *r) {
    SDL_Rect bg = { 0, 0, 1280, 720 };
    SDL_Color white = { 240, 245, 255, 255 };
    SDL_Color cyan = { 77, 226, 195, 255 };
    if (!title_font) title_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 46);
    if (!body_font) body_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 24);
    SDL_SetRenderDrawColor(r, 10, 18, 31, 255);
    SDL_RenderFillRect(r, &bg);
    text(r, title_font, "전공 아이템 가이드", 430, 75, cyan);
    text(r, body_font, "패킷 송신     안정적인 기본 공격 (18~28)", 355, 200, white);
    text(r, body_font, "방화벽         2턴 동안 받는 피해를 50% 감소", 355, 270, white);
    text(r, body_font, "디버거         오류를 찾아 강한 피해 (2회)", 355, 340, white);
    text(r, body_font, "에너지 드링크  체력 32 회복 (2개)", 355, 410, white);
    text(r, body_font, "아무 키나 누르면 돌아갑니다.", 455, 570, cyan);
}

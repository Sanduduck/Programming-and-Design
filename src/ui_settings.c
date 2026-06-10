#include <SDL_ttf.h>

#include "game_state.h"
#include "ui_settings.h"

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

void handle_settings_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN || e->type == SDL_MOUSEBUTTONDOWN)
        change_state(STATE_MAIN_MENU);
}

void update_settings(float dt) {
    (void)dt;
}

void draw_settings(SDL_Renderer *r) {
    SDL_Rect bg = { 0, 0, 1280, 720 };
    SDL_Color white = { 240, 245, 255, 255 };
    SDL_Color cyan = { 77, 226, 195, 255 };
    if (!title_font) title_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 46);
    if (!body_font) body_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 25);
    SDL_SetRenderDrawColor(r, 10, 18, 31, 255);
    SDL_RenderFillRect(r, &bg);
    text(r, title_font, "조작 방법", 520, 90, cyan);
    text(r, body_font, "숫자 1~4: 전투 행동 선택", 430, 220, white);
    text(r, body_font, "← / →: 버튼 이동, Enter/Space: 결정", 430, 280, white);
    text(r, body_font, "마우스: 전투 버튼 클릭", 430, 340, white);
    text(r, body_font, "M 또는 ESC: 메인 메뉴", 430, 400, white);
    text(r, body_font, "아무 키나 누르면 돌아갑니다.", 455, 560, cyan);
}

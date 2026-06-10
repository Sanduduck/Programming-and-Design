#include <SDL_ttf.h>
#include <stdio.h>

#include "game_state.h"
#include "ui_menu.h"

#define WINDOW_W 1280
#define WINDOW_H 720

static TTF_Font *title_font;
static TTF_Font *menu_font;
static TTF_Font *small_font;

static void load_fonts(void) {
    if (!title_font) title_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 52);
    if (!menu_font) menu_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 28);
    if (!small_font) small_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 20);
}

static void draw_text(SDL_Renderer *r, TTF_Font *font, const char *text,
                      int x, int y, SDL_Color color) {
    SDL_Surface *surface = font ? TTF_RenderUTF8_Blended(font, text, color) : NULL;
    if (surface) {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(r, surface);
        SDL_Rect dst = { x, y, surface->w, surface->h };
        if (texture) SDL_RenderCopy(r, texture, NULL, &dst);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
}

static int menu_at(int x, int y) {
    int i;
    for (i = 0; i < 3; i++) {
        SDL_Rect box = { 470, 370 + i * 85, 340, 60 };
        if (x >= box.x && x < box.x + box.w && y >= box.y && y < box.y + box.h) return i;
    }
    return -1;
}

void handle_menu_event(SDL_Event *e) {
    int item = -1;
    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_1) item = 0;
        else if (e->key.keysym.sym == SDLK_2) item = 1;
        else if (e->key.keysym.sym == SDLK_3) item = 2;
        else if (e->key.keysym.sym == SDLK_RETURN) item = 0;
    } else if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        item = menu_at(e->button.x, e->button.y);
    }
    if (item == 0) change_state(STATE_PLAYING);
    else if (item == 1) change_state(STATE_RANKING);
    else if (item == 2) change_state(STATE_SETTINGS);
}

void update_menu(float dt) {
    (void)dt;
}

void draw_menu(SDL_Renderer *r) {
    SDL_Color white = { 242, 246, 255, 255 };
    SDL_Color cyan = { 82, 226, 198, 255 };
    const char *labels[3] = { "1. 게임 시작", "2. 전공 가이드", "3. 조작 방법" };
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    int i;
    load_fonts();

    SDL_SetRenderDrawColor(r, 10, 18, 31, 255);
    SDL_RenderFillRect(r, &bg);
    SDL_SetRenderDrawColor(r, 25, 48, 69, 255);
    for (i = 0; i < 11; i++) {
        SDL_RenderDrawLine(r, 0, 70 + i * 55, WINDOW_W, 70 + i * 55);
    }
    for (i = 0; i < 17; i++) {
        SDL_RenderDrawLine(r, 40 + i * 75, 0, 40 + i * 75, WINDOW_H);
    }

    draw_text(r, title_font, "정보통신공학과", 445, 105, cyan);
    draw_text(r, title_font, "캡스톤 퀘스트", 445, 175, white);
    draw_text(r, small_font, "패킷과 코드로 과제 보스를 물리치는 턴제 RPG", 432, 265, white);

    for (i = 0; i < 3; i++) {
        SDL_Rect button = { 470, 370 + i * 85, 340, 60 };
        SDL_SetRenderDrawColor(r, 24, 44, 68, 255);
        SDL_RenderFillRect(r, &button);
        SDL_SetRenderDrawColor(r, cyan.r, cyan.g, cyan.b, 255);
        SDL_RenderDrawRect(r, &button);
        draw_text(r, menu_font, labels[i], button.x + 75, button.y + 12, white);
    }
    draw_text(r, small_font, "ESC: 종료", 590, 660, white);
}

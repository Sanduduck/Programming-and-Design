#include <SDL_ttf.h>
#include "ui_menu.h"
#include "game_state.h"

#define WINDOW_W 1280
#define WINDOW_H 720
#define BOX_W 320
#define BOX_H 70
#define BOX_X (WINDOW_W / 2 - BOX_W / 2)
#define BOX_START_Y 400

static TTF_Font *menu_font = NULL;

static void load_font(void) {
    if (menu_font) return;

    menu_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 28);
}

static void draw_text_centered(SDL_Renderer *r, const char *text,
                               int box_x, int box_y, int box_w, int box_h) {
    if (!menu_font) return;

    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Surface *surf = TTF_RenderUTF8_Blended(menu_font, text, white);
    if (!surf) return;

    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        SDL_Rect dst = {
            box_x + (box_w - surf->w) / 2,
            box_y + (box_h - surf->h) / 2,
            surf->w,
            surf->h
        };
        SDL_RenderCopy(r, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

static void activate_selected_menu(void) {
    change_state(STATE_PLAYING);
}

void handle_menu_event(SDL_Event *e) {
    if (e->type != SDL_KEYDOWN) return;

    switch (e->key.keysym.sym) {
        case SDLK_RETURN:
        case SDLK_SPACE:
        case SDLK_1:
        case SDLK_KP_1:
            activate_selected_menu();
            break;
    }
}

void update_menu(float dt) {
    (void)dt;
}

void draw_menu(SDL_Renderer *r) {
    load_font();

    // 처음 들어가자마자 보이는 메인 메뉴 화면을 그리는 함수이다.
    // game_state.c의 current_state가 STATE_MAIN_MENU일 때 이 함수가 호출된다.
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    SDL_RenderFillRect(r, &bg);

    // 화면 위쪽의 게임 제목 박스와 제목 글씨를 그린다.
    SDL_SetRenderDrawColor(r, 230, 220, 100, 255);
    SDL_Rect title = { WINDOW_W / 2 - 200, 120, 400, 80 };
    SDL_RenderFillRect(r, &title);
    draw_text_centered(r, "SURVICE ICE", title.x, title.y, title.w, title.h);

    // 가운데의 시작 버튼 영역을 그린다.
    SDL_Rect start_box = { BOX_X, BOX_START_Y, BOX_W, BOX_H };
    SDL_SetRenderDrawColor(r, 70, 130, 180, 255);
    SDL_RenderFillRect(r, &start_box);

    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderDrawRect(r, &start_box);
    draw_text_centered(r, "1. Start", start_box.x, start_box.y,
                       start_box.w, start_box.h);

    // 화면 아래쪽에 시작 방법 안내 문구를 그린다.
    draw_text_centered(r, "Enter or Space: Start",
                       0, 650, WINDOW_W, 40);
}

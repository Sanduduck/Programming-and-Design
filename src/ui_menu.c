// 메인 메뉴 화면

#include <SDL_ttf.h>
#include "ui_menu.h"
#include "game_state.h"

// 메뉴 박스 좌표 (draw / 마우스 클릭 둘 다 공유)
#define BOX_W       280
#define BOX_H       60
#define BOX_X       (WINDOW_W / 2 - BOX_W / 2)
#define BOX_START_Y 400
#define BOX_GAP     90

static TTF_Font *menu_font = NULL;

// 폰트 처음 한 번만 로드 (Windows 시스템 폰트 사용)
static void load_font(void) {
    if (menu_font) return;
    menu_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 28);
}

// 사각형 영역 안에 텍스트 가운데 정렬해서 그리기
static void draw_text_centered(SDL_Renderer *r, const char *text,
                               int box_x, int box_y, int box_w, int box_h) {
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Surface *surf = TTF_RenderUTF8_Blended(menu_font, text, white);
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

// 마우스 좌표가 i번째 박스 안에 있는지
static int is_in_box(int mx, int my, int i) {
    int by = BOX_START_Y + BOX_GAP * i;
    return (mx >= BOX_X && mx < BOX_X + BOX_W &&
            my >= by      && my < by + BOX_H);
}

void handle_menu_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_1:
            case SDLK_KP_1:
                change_state(STATE_PLAYING);
                break;
            case SDLK_2:
            case SDLK_KP_2:
                change_state(STATE_RANKING);
                break;
            case SDLK_3:
            case SDLK_KP_3:
                change_state(STATE_SETTINGS);
                break;
        }
    } else if (e->type == SDL_MOUSEBUTTONDOWN &&
               e->button.button == SDL_BUTTON_LEFT) {
        int mx = e->button.x;
        int my = e->button.y;
        if      (is_in_box(mx, my, 0)) change_state(STATE_PLAYING);
        else if (is_in_box(mx, my, 1)) change_state(STATE_RANKING);
        else if (is_in_box(mx, my, 2)) change_state(STATE_SETTINGS);
    }
}

void update_menu(float dt) {
    (void)dt;
}

void draw_menu(SDL_Renderer *r) {
    load_font();

    // 검은 배경
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    SDL_RenderFillRect(r, &bg);

    // 타이틀 자리 (노란 사각형 - 추후 ASCII 아트 자리)
    SDL_SetRenderDrawColor(r, 230, 220, 100, 255);
    SDL_Rect title = { WINDOW_W / 2 - 200, 120, 400, 80 };
    SDL_RenderFillRect(r, &title);

    // 메뉴 항목 사각형 (흰 테두리만) + 흰 텍스트
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);

    SDL_Rect items[3] = {
        { BOX_X, BOX_START_Y,               BOX_W, BOX_H },
        { BOX_X, BOX_START_Y + BOX_GAP,     BOX_W, BOX_H },
        { BOX_X, BOX_START_Y + BOX_GAP * 2, BOX_W, BOX_H }
    };
    const char *labels[3] = { "1.시작", "2.랭킹", "3.설정" };

    for (int i = 0; i < 3; i++) {
        SDL_RenderDrawRect(r, &items[i]);
        draw_text_centered(r, labels[i],
                           items[i].x, items[i].y,
                           items[i].w, items[i].h);
    }
}

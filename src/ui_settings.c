// 설정 화면: 키 바인딩, 볼륨 (추후)

#include "ui_settings.h"
#include "game_state.h"

#define WINDOW_W 1280
#define WINDOW_H 720

void handle_settings_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_RETURN ||
            e->key.keysym.sym == SDLK_m) {
            change_state(STATE_MAIN_MENU);
        }
    }
}

void update_settings(float dt) {
    (void)dt;
}

void draw_settings(SDL_Renderer *r) {
    // 검은 배경
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    SDL_RenderFillRect(r, &bg);
}

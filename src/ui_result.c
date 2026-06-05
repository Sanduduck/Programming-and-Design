// 결과 화면: 점수 + 학점 + 재시작/메뉴 복귀

#include "ui_result.h"
#include "game_state.h"

void handle_result_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_RETURN) {
            change_state(STATE_PLAYING);
        } else if (e->key.keysym.sym == SDLK_ESCAPE) {
            change_state(STATE_MAIN_MENU);
        }
    }
}

void update_result(float dt) {
    (void)dt;
}

void draw_result(SDL_Renderer *r) {
    // 검은 배경
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    SDL_RenderFillRect(r, &bg);
}

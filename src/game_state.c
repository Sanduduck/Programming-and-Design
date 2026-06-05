#include "game_state.h"
#include "ui_menu.h"
#include "ui_play.h"
#include "player.h"
#include "pattern.h"

// 게임을 실행하면 처음에는 메인 메뉴 화면부터 보이도록 시작 상태를 설정한다.
GameState current_state = STATE_MAIN_MENU;

void init_game(void) {
    init_patterns();
    init_player();
}

void shutdown_game(void) {
}

void handle_event(SDL_Event *e) {
    switch (current_state) {
        case STATE_MAIN_MENU: handle_menu_event(e); break;
        case STATE_PLAYING:   handle_play_event(e); break;
    }
}

void update_game(float dt) {
    switch (current_state) {
        case STATE_MAIN_MENU: update_menu(dt); break;
        case STATE_PLAYING:   update_play(dt); break;
    }
}

void draw_game(SDL_Renderer *r) {
    // 현재 게임 상태에 따라 어떤 화면을 그릴지 결정한다.
    // 처음 실행 시 current_state가 STATE_MAIN_MENU이므로 draw_menu()가 호출된다.
    switch (current_state) {
        case STATE_MAIN_MENU: draw_menu(r); break;
        case STATE_PLAYING:   draw_play(r); break;
    }
}

void change_state(GameState next) {
    if (next == STATE_PLAYING) {
        init_player();
        start_pattern();
    }
    current_state = next;
}

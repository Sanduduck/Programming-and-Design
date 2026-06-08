// 게임 상태 머신: 각 화면별 이벤트/업데이트/렌더 디스패치

#include "game_state.h"
#include "ui_menu.h"
#include "ui_play.h"
#include "ui_result.h"
#include "ui_ranking.h"
#include "ui_settings.h"
#include "player.h"
#include "settings.h"
#include "audio.h"

GameState current_state = STATE_MAIN_MENU;

void init_game(void) {
    init_settings();
    init_player();
    audio_init();
}

void shutdown_game(void) {
    audio_shutdown();
    // 종료 시 정리할 것 있으면 여기에
}

void handle_event(SDL_Event *e) {
    switch (current_state) {
        case STATE_MAIN_MENU: handle_menu_event(e); break;
        case STATE_PLAYING:   handle_play_event(e); break;
        case STATE_RESULT:    handle_result_event(e); break;
        case STATE_RANKING:   handle_ranking_event(e); break;
        case STATE_SETTINGS:  handle_settings_event(e); break;
    }
}

void update_game(float dt) {
    switch (current_state) {
        case STATE_MAIN_MENU: update_menu(dt); break;
        case STATE_PLAYING:   update_play(dt); break;
        case STATE_RESULT:    update_result(dt); break;
        case STATE_RANKING:   update_ranking(dt); break;
        case STATE_SETTINGS:  update_settings(dt); break;
    }
}

void draw_game(SDL_Renderer *r) {
    switch (current_state) {
        case STATE_MAIN_MENU: draw_menu(r); break;
        case STATE_PLAYING:   draw_play(r); break;
        case STATE_RESULT:    draw_result(r); break;
        case STATE_RANKING:   draw_ranking(r); break;
        case STATE_SETTINGS:  draw_settings(r); break;
    }
}

void change_state(GameState next) {
    // 게임 진입 시 플레이어 초기화
    if (next == STATE_PLAYING) {
        init_player();
    }
    current_state = next;
}

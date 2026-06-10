#include "game_state.h"
#include "player.h"
#include "score.h"
#include "ui_menu.h"
#include "ui_play.h"
#include "ui_ranking.h"
#include "ui_result.h"
#include "ui_settings.h"

GameState current_state = STATE_MAIN_MENU;

void init_game(void) {
    init_player();
    init_score();
}

void shutdown_game(void) {
}

void handle_event(SDL_Event *e) {
    switch (current_state) {
        case STATE_MAIN_MENU: handle_menu_event(e); break;
        case STATE_PLAYING: handle_play_event(e); break;
        case STATE_RESULT: handle_result_event(e); break;
        case STATE_RANKING: handle_ranking_event(e); break;
        case STATE_SETTINGS: handle_settings_event(e); break;
    }
}

void update_game(float dt) {
    switch (current_state) {
        case STATE_MAIN_MENU: update_menu(dt); break;
        case STATE_PLAYING: update_play(dt); break;
        case STATE_RESULT: update_result(dt); break;
        case STATE_RANKING: update_ranking(dt); break;
        case STATE_SETTINGS: update_settings(dt); break;
    }
}

void draw_game(SDL_Renderer *r) {
    switch (current_state) {
        case STATE_MAIN_MENU: draw_menu(r); break;
        case STATE_PLAYING: draw_play(r); break;
        case STATE_RESULT: draw_result(r); break;
        case STATE_RANKING: draw_ranking(r); break;
        case STATE_SETTINGS: draw_settings(r); break;
    }
}

void change_state(GameState next) {
    if (next == STATE_PLAYING) {
        init_player();
        init_score();
        init_battle();
    }
    current_state = next;
}

// 게임 전역 설정: 볼륨 (마스터/BGM/효과음) + 조작 방식 (방향키/WASD)

#include <SDL.h>
#include "settings.h"

Settings g_settings;

void init_settings(void) {
    g_settings.master_volume = 100;
    g_settings.bgm_volume    = 70;
    g_settings.sfx_volume    = 80;
    g_settings.god_mode      = false;
    settings_set_scheme(CONTROL_ARROWS);
}

void settings_set_scheme(ControlScheme scheme) {
    g_settings.control_scheme = scheme;
    if (scheme == CONTROL_WASD) {
        g_settings.key_left  = SDL_SCANCODE_A;
        g_settings.key_right = SDL_SCANCODE_D;
        g_settings.key_jump  = SDL_SCANCODE_W;
    } else {
        g_settings.key_left  = SDL_SCANCODE_LEFT;
        g_settings.key_right = SDL_SCANCODE_RIGHT;
        g_settings.key_jump  = SDL_SCANCODE_UP;
    }
}

int settings_effective_bgm(void) {
    return g_settings.master_volume * g_settings.bgm_volume / 100;
}

int settings_effective_sfx(void) {
    return g_settings.master_volume * g_settings.sfx_volume / 100;
}

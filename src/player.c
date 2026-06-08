// 플레이어: 좌우 이동, 2단 점프, 중력, 바닥 충돌

#include "player.h"
#include "settings.h"
#include "audio.h"

#define MOVE_SPEED     350.0f    // 좌우 이동 속도 (px/s)
#define JUMP_SPEED     650.0f    // 점프 초속도 (px/s)
#define GRAVITY        1600.0f   // 중력 가속도 (px/s^2)

Player player;

void init_player(void) {
    player.x = 100.0f;
    player.y = (float)(FLOOR_Y - PLAYER_H);
    player.vx = 0.0f;
    player.vy = 0.0f;
    player.hp = 4;
    player.jump_count = 0;
    player.max_jumps = 2;
    player.invincible_timer = 0;
    player.on_ground = true;
}

void update_player(float dt) {
    // 좌우 입력 폴링 — 설정 화면에서 선택한 조작 방식의 키만 사용
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    player.vx = 0.0f;
    if (keys[g_settings.key_left])  player.vx = -MOVE_SPEED;
    if (keys[g_settings.key_right]) player.vx = MOVE_SPEED;

    // 중력
    player.vy += GRAVITY * dt;

    // 위치 갱신
    player.x += player.vx * dt;
    player.y += player.vy * dt;

    // 좌우 화면 경계
    if (player.x < 0) player.x = 0;
    if (player.x + PLAYER_W > WINDOW_W) player.x = (float)(WINDOW_W - PLAYER_W);

    // 바닥 충돌 처리
    if (player.y + PLAYER_H >= FLOOR_Y) {
        player.y = (float)(FLOOR_Y - PLAYER_H);
        player.vy = 0.0f;
        player.on_ground = true;
        player.jump_count = 0;
    } else {
        player.on_ground = false;
    }

    // 무적 타이머 감소
    if (player.invincible_timer > 0) player.invincible_timer--;
}

void draw_player(SDL_Renderer *r) {
    // 무적 중이면 6프레임 간격으로 안 그림 → 깜빡임 효과
    if (player.invincible_timer > 0 && (player.invincible_timer / 6) % 2 == 0) {
        return;
    }
    // 임시 사각형 (나중에 스프라이트로 교체)
    SDL_Rect rect = { (int)player.x, (int)player.y, (int)PLAYER_W, (int)PLAYER_H };
    SDL_SetRenderDrawColor(r, 135, 206, 235, 255);   // 하늘색
    SDL_RenderFillRect(r, &rect);
}

void player_jump(void) {
    // 2단 점프: 첫 점프든 공중 점프든 vy 초기화 후 위로
    if (player.jump_count < player.max_jumps) {
        player.vy = -JUMP_SPEED;
        player.jump_count++;
        player.on_ground = false;
        audio_play_jump();
    }
}

void player_damage(void) {
    if (g_settings.god_mode) return;
    if (player.invincible_timer > 0) return;
    if (player.hp > 0) player.hp--;
    player.invincible_timer = 60;   // 임시로 1초 (60프레임 @ 60FPS)
}

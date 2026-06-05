#include "player.h"

#define BOX_X 240
#define BOX_Y 120
#define BOX_W 800
#define FLOOR_Y 600

#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 50
#define MOVE_SPEED 350.0f
#define JUMP_POWER 650.0f
#define GRAVITY 1600.0f
#define INVINCIBLE_TIME 18

Player player;

// 플레이어의 시작 위치, 체력, 점프 상태를 초기화한다.
void init_player(void) {
    player.x = (float)(BOX_X + BOX_W / 2 - PLAYER_WIDTH / 2);
    player.y = (float)(FLOOR_Y - PLAYER_HEIGHT);
    player.vx = 0.0f;
    player.vy = 0.0f;
    player.hp = 4;
    player.jump_count = 0;
    player.max_jumps = 2;
    player.invincible_timer = 0;
    player.on_ground = true;
}

// 매 프레임 키 입력, 중력, 이동 범위 제한, 바닥 착지를 처리한다.
void update_player(float dt) {
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    // A/D 입력에 따라 좌우 이동 속도를 정한다.
    player.vx = 0.0f;
    if (keys[SDL_SCANCODE_A]) {
        player.vx = -MOVE_SPEED;
    }
    if (keys[SDL_SCANCODE_D]) {
        player.vx = MOVE_SPEED;
    }

    // 중력을 적용한 뒤 속도에 따라 위치를 갱신한다.
    player.vy += GRAVITY * dt;
    player.x += player.vx * dt;
    player.y += player.vy * dt;

    // 플레이어가 게임 박스 밖으로 나가지 않도록 위치를 보정한다.
    if (player.x < BOX_X) {
        player.x = (float)BOX_X;
    }
    if (player.x + PLAYER_WIDTH > BOX_X + BOX_W) {
        player.x = (float)(BOX_X + BOX_W - PLAYER_WIDTH);
    }
    if (player.y < BOX_Y) {
        player.y = (float)BOX_Y;
        player.vy = 0.0f;
    }

    // 바닥에 닿으면 착지 상태로 바꾸고 점프 횟수를 초기화한다.
    if (player.y + PLAYER_HEIGHT >= FLOOR_Y) {
        player.y = (float)(FLOOR_Y - PLAYER_HEIGHT);
        player.vy = 0.0f;
        player.on_ground = true;
        player.jump_count = 0;
    } else {
        player.on_ground = false;
    }

    // 피격 후 일정 시간 동안 무적 상태를 유지한다.
    if (player.invincible_timer > 0) {
        player.invincible_timer--;
    }
}

// 무적 시간 중에는 깜빡이게 그리고, 평소에는 사각형 플레이어를 그린다.
void draw_player(SDL_Renderer *r) {
    if (player.invincible_timer > 0 &&
        (player.invincible_timer / 3) % 2 == 0) {
        return;
    }

    SDL_Rect rect = {
        (int)player.x,
        (int)player.y,
        PLAYER_WIDTH,
        PLAYER_HEIGHT
    };
    SDL_SetRenderDrawColor(r, 135, 206, 235, 255);
    SDL_RenderFillRect(r, &rect);
}

// 최대 점프 횟수 안에서 점프를 실행한다. 현재 설정은 2단 점프이다.
void player_jump(void) {
    if (player.jump_count < player.max_jumps) {
        player.vy = -JUMP_POWER;
        player.jump_count++;
        player.on_ground = false;
    }
}

// 데미지를 받으면 체력을 줄이고, 연속 피격 방지를 위해 무적 시간을 준다.
void player_damage(void) {
    if (player.invincible_timer > 0) return;

    if (player.hp > 0) {
        player.hp--;
    }
    player.invincible_timer = INVINCIBLE_TIME;
}

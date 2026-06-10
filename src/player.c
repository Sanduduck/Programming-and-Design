#include "player.h"

Player player;

void init_player(void) {
    player.x = 640.0f;
    player.y = 410.0f;
    player.vx = 0.0f;
    player.vy = 0.0f;
    player.hp = 100;
    player.jump_count = 0;
    player.max_jumps = 0;
    player.invincible_timer = 0;
    player.on_ground = true;
}

// 충돌 판정 (AABB)

#include "collision.h"
#include "player.h"
#include "obstacle.h"

bool check_collision(const Player *p, const Obstacle *o) {
    // 임시 플레이어 크기 (40 x 60)
    int px = (int)p->x, py = (int)p->y;
    int pw = 40, ph = 60;
    int ox = (int)o->x, oy = (int)o->y;
    int ow = o->width, oh = o->height;

    if (px + pw <= ox) return false;
    if (px >= ox + ow) return false;
    if (py + ph <= oy) return false;
    if (py >= oy + oh) return false;
    return true;
}

void check_all_collisions(void) {
    if (player.invincible_timer > 0) return;
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;
        if (check_collision(&player, &obstacles[i])) {
            player_damage();
            obstacles[i].scored = true; // 피격된 장애물은 점수 부여 X
            break;
        }
    }
}

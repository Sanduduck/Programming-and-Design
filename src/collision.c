#include "collision.h"
#include "player.h"
#include "obstacle.h"

bool check_collision(const Player *p, const Obstacle *o) {
    // AABB 충돌 검사 방식이다.
    // 두 물체를 사각형으로 보고, x/y 범위가 서로 겹치면 충돌로 처리한다.
    int px = (int)p->x;
    int py = (int)p->y;
    int pw = 50;
    int ph = 50;

    int ox = (int)o->x;
    int oy = (int)o->y;
    int ow = o->width;
    int oh = o->height;

    if (px + pw <= ox) return false; // 플레이어가 장애물 왼쪽에 있음
    if (px >= ox + ow) return false; // 플레이어가 장애물 오른쪽에 있음
    if (py + ph <= oy) return false; // 플레이어가 장애물 위쪽에 있음
    if (py >= oy + oh) return false; // 플레이어가 장애물 아래쪽에 있음
    return true;
}

void check_all_collisions(void) {
    // 무적 시간 중이면 충돌해도 피해를 받지 않는다.
    if (player.invincible_timer > 0) return;

    // 현재 활성화된 장애물을 하나씩 검사한다.
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;

        if (check_collision(&player, &obstacles[i])) {
            player_damage();
            break;
        }
    }
}

 #include "obstacle.h"

Obstacle obstacles[MAX_OBSTACLES];

void init_obstacles(void) {
    // 배열에 들어 있는 모든 장애물을 비활성 상태로 만든다.
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].active = false;
    }
}

void spawn_obstacle(float x, float y, float vx, float vy,
                    int w, int h, int pattern_id) {
    // 비어 있는 칸을 찾아서 새 장애물을 넣는다.
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) {
            obstacles[i].x = x;
            obstacles[i].y = y;
            obstacles[i].vx = vx;
            obstacles[i].vy = vy;
            obstacles[i].width = w;
            obstacles[i].height = h;
            obstacles[i].active = true;
            obstacles[i].pattern_id = pattern_id;
            return;
        }
    }
}

void update_obstacles(float dt) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;

        // 속도에 시간 차이를 곱해서 위치를 이동시킨다.
        obstacles[i].x += obstacles[i].vx * dt;
        obstacles[i].y += obstacles[i].vy * dt;

        // 화면 밖으로 충분히 멀리 나간 장애물은 다시 사용 가능하게 비활성화한다.
        if (obstacles[i].y + obstacles[i].height < -80 ||
            obstacles[i].y > 800 ||
            obstacles[i].x + obstacles[i].width < -120 ||
            obstacles[i].x > 1400) {
            obstacles[i].active = false;
        }
    }
}

static void draw_signal(SDL_Renderer *r, SDL_Rect rect) {
    // 기지국 패턴의 전파 장애물이다.
    // 노란 막대에 선을 추가해서 전파 느낌을 낸다.
    SDL_SetRenderDrawColor(r, 255, 220, 80, 255);
    SDL_RenderFillRect(r, &rect);

    SDL_SetRenderDrawColor(r, 255, 255, 170, 255);
    SDL_RenderDrawRect(r, &rect);
    SDL_RenderDrawLine(r, rect.x + 8, rect.y + rect.h / 2,
                       rect.x + rect.w - 8, rect.y + rect.h / 2);
    SDL_RenderDrawLine(r, rect.x + 18, rect.y + 4,
                       rect.x + rect.w - 18, rect.y + rect.h - 4);
    SDL_RenderDrawLine(r, rect.x + 18, rect.y + rect.h - 4,
                       rect.x + rect.w - 18, rect.y + 4);
}

static void draw_virus(SDL_Renderer *r, SDL_Rect rect) {
    // 바이러스 패턴의 장애물이다.
    // 초록 사각형에 돌기와 눈을 붙여 간단히 표현했다.
    SDL_SetRenderDrawColor(r, 80, 220, 120, 255);
    SDL_RenderFillRect(r, &rect);

    SDL_SetRenderDrawColor(r, 20, 120, 50, 255);
    SDL_RenderDrawRect(r, &rect);

    int center_x = rect.x + rect.w / 2;
    int center_y = rect.y + rect.h / 2;
    SDL_RenderDrawLine(r, rect.x - 8, center_y, rect.x, center_y);
    SDL_RenderDrawLine(r, rect.x + rect.w, center_y, rect.x + rect.w + 8, center_y);
    SDL_RenderDrawLine(r, center_x, rect.y - 8, center_x, rect.y);
    SDL_RenderDrawLine(r, center_x, rect.y + rect.h, center_x, rect.y + rect.h + 8);

    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_Rect eye_left = { rect.x + rect.w / 4, rect.y + rect.h / 3, 5, 5 };
    SDL_Rect eye_right = { rect.x + rect.w * 3 / 4 - 5, rect.y + rect.h / 3, 5, 5 };
    SDL_RenderFillRect(r, &eye_left);
    SDL_RenderFillRect(r, &eye_right);
}

void draw_obstacles(SDL_Renderer *r) {
    // obstacles 배열을 돌면서 현재 활성화된 장애물만 화면에 그린다.
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;

        // 장애물의 위치와 크기를 SDL에서 그릴 수 있는 사각형 형태로 변환한다.
        SDL_Rect rect = {
            (int)obstacles[i].x,
            (int)obstacles[i].y,
            obstacles[i].width,
            obstacles[i].height
        };

        // pattern_id에 따라 장애물의 모양을 다르게 그린다.
        if (obstacles[i].pattern_id == 410) {
            draw_signal(r, rect);
        } else {
            draw_virus(r, rect);
        }
    }
}

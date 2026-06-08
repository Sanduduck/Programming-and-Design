// 정보보안(바이러스) 패턴: 좌 → / 우 ← / 위 ↓ 세 방향에서 바이러스가 들어온다.
// 자체 인스턴스 배열 + 자체 충돌 판정 보유 (공용 obstacle 모듈에 의존하지 않음).

#include "virus.h"
#include "../player.h"
#include <stdlib.h>

#define MAX_VIRUS 48
#define VIRUS_SIZE 42

// 바이러스는 화면 끝(벽)에서 출발한다.
#define EDGE_L (-(float)VIRUS_SIZE)   // 왼쪽 벽 바깥
#define EDGE_R ((float)WINDOW_W)      // 오른쪽 벽
#define EDGE_T (-(float)VIRUS_SIZE)   // 위쪽 벽 바깥
#define EDGE_B ((float)WINDOW_H)      // 아래쪽 벽
#define VIRUS_VX 330.0f               // 좌우 이동 속력 (px/s)
#define VIRUS_VY 300.0f               // 상하 이동 속력 (px/s)

// 화면에 떠 있는 바이러스 한 마리의 상태
typedef struct {
    float x, y, vx, vy;
    bool active;
} Virus;

// 웨이브 스폰 파라미터 — 웨이브마다 좌/우/위/아래에서 한 마리씩(총 4마리)
#define WAVE_INTERVAL 0.7f   // 웨이브 간격(초)
#define TOTAL_WAVES   6      // 총 웨이브 수

static Virus viruses[MAX_VIRUS];
static float pattern_time = 0.0f;
static int waves_spawned = 0;

void virus_start(void) {
    pattern_time = 0.0f;
    waves_spawned = 0;
    for (int i = 0; i < MAX_VIRUS; i++) {
        viruses[i].active = false;
    }
}

static void spawn_virus(float x, float y, float vx, float vy) {
    for (int i = 0; i < MAX_VIRUS; i++) {
        if (!viruses[i].active) {
            viruses[i].x = x;
            viruses[i].y = y;
            viruses[i].vx = vx;
            viruses[i].vy = vy;
            viruses[i].active = true;
            return;
        }
    }
}

// 사각(x,y,VIRUS_SIZE) 과 플레이어 AABB 충돌
static bool hit_player(float x, float y) {
    return !(x + VIRUS_SIZE < player.x || x > player.x + PLAYER_W ||
             y + VIRUS_SIZE < player.y || y > player.y + PLAYER_H);
}

// (x,y) 자리가 기존 활성 바이러스와 겹치는지 — 크기 + 간격(12) 포함
static bool overlaps_active(float x, float y) {
    for (int i = 0; i < MAX_VIRUS; i++) {
        if (!viruses[i].active) continue;
        float dx = x - viruses[i].x; if (dx < 0) dx = -dx;
        float dy = y - viruses[i].y; if (dy < 0) dy = -dy;
        if (dx < VIRUS_SIZE + 12 && dy < VIRUS_SIZE + 12) return true;
    }
    return false;
}

typedef enum { DIR_L, DIR_R, DIR_T, DIR_B } VirusDir;

// 지정한 방향(좌/우/위/아래) 벽에서, 겹치지 않는 자리를 찾으면 스폰
static void spawn_dir(VirusDir dir) {
    for (int attempt = 0; attempt < 12; attempt++) {
        float x = 0.0f, y = 0.0f, vx = 0.0f, vy = 0.0f;
        switch (dir) {
            case DIR_L: x = EDGE_L;             y = 180 + rand() % 340; vx =  VIRUS_VX; break;
            case DIR_R: x = EDGE_R;             y = 180 + rand() % 340; vx = -VIRUS_VX; break;
            case DIR_T: x = 200 + rand() % 880; y = EDGE_T;             vy =  VIRUS_VY; break;
            case DIR_B: x = 200 + rand() % 880; y = EDGE_B;             vy = -VIRUS_VY; break;
        }
        if (!overlaps_active(x, y)) {
            spawn_virus(x, y, vx, vy);
            return;
        }
    }
}

void virus_update(float dt) {
    pattern_time += dt;

    // 0.3초 후부터 WAVE_INTERVAL 간격으로 한 웨이브씩, 좌/우/위/아래에서 한 마리씩
    while (waves_spawned < TOTAL_WAVES &&
           pattern_time >= 0.3f + waves_spawned * WAVE_INTERVAL) {
        spawn_dir(DIR_L);
        spawn_dir(DIR_R);
        spawn_dir(DIR_T);
        spawn_dir(DIR_B);
        waves_spawned++;
    }

    for (int i = 0; i < MAX_VIRUS; i++) {
        if (!viruses[i].active) continue;

        viruses[i].x += viruses[i].vx * dt;
        viruses[i].y += viruses[i].vy * dt;

        if (hit_player(viruses[i].x, viruses[i].y)) {
            player_damage();
        }

        // 화면 밖으로 충분히 벗어나면 슬롯 회수
        if (viruses[i].x < -120 || viruses[i].x > WINDOW_W + 120 ||
            viruses[i].y < -120 || viruses[i].y > WINDOW_H + 120) {
            viruses[i].active = false;
        }
    }
}

bool virus_finished(void) {
    if (waves_spawned < TOTAL_WAVES) return false;
    for (int i = 0; i < MAX_VIRUS; i++) {
        if (viruses[i].active) return false;
    }
    return true;
}

// 바이러스 한 마리: 초록 사각형 + 돌기 + 눈
static void draw_virus(SDL_Renderer *r, SDL_Rect rect) {
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

void virus_draw(SDL_Renderer *r) {
    for (int i = 0; i < MAX_VIRUS; i++) {
        if (!viruses[i].active) continue;
        SDL_Rect rect = { (int)viruses[i].x, (int)viruses[i].y, VIRUS_SIZE, VIRUS_SIZE };
        draw_virus(r, rect);
    }
}

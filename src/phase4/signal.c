// 이동통신(신호) 패턴: 좌 → / 우 ← 두 방향에서 전파 막대가 들어온다.
// 자체 인스턴스 배열 + 자체 충돌 판정 보유 (공용 obstacle 모듈에 의존하지 않음).
// 플레이 영역 양쪽에는 송신소(기지국) 장식을 세운다.

#include "signal.h"
#include "../player.h"
#include <math.h>

#define MAX_SIGNAL 16
#define SIGNAL_W 90
#define SIGNAL_H 30
#define HOMING_TIME 0.25f    // 스폰 직후 플레이어를 추격하는 시간(초)
#define SIGNAL_SPEED 240.0f  // 추격/직진 속력 (px/s)

// 송신소(기지국) 치수 — 몸체 아랫면을 바닥(FLOOR_Y)에 맞춰 세운다.
#define STATION_W      88
#define STATION_BODY_H 250
#define STATION_MAST_H 70    // 몸체 위로 솟은 안테나 기둥 높이
#define STATION_MARGIN 15    // 화면 벽과의 간격
#define STATION_LEFT_X  STATION_MARGIN
#define STATION_RIGHT_X (WINDOW_W - STATION_W - STATION_MARGIN)

// 안테나 송출점(전파가 갈라져 나오는 지점) — 막대 스폰 시작점
#define EMIT_Y       (FLOOR_Y - STATION_BODY_H - STATION_MAST_H)
#define LEFT_EMIT_X  (STATION_LEFT_X + STATION_W / 2)
#define RIGHT_EMIT_X (STATION_RIGHT_X + STATION_W / 2)

typedef struct {
    float x, y, vx, vy;
    float age;    // 스폰 후 경과 시간 — HOMING_TIME 동안 플레이어 추격
    bool active;
} Signal;

typedef struct {
    float spawn_time;
    float x, y, vx;
} SignalSpawn;

static Signal signals[MAX_SIGNAL];
static float pattern_time = 0.0f;
static int next_spawn = 0;

// 좌→ / 우← 두 방향 웨이브를 시간차로 반복. 약 4초.
// 막대는 좌/우 기지국 안테나 송출점에서 출발해 곧바로 플레이어를 추격한다.
#define L_X (LEFT_EMIT_X - SIGNAL_W / 2)
#define R_X (RIGHT_EMIT_X - SIGNAL_W / 2)
#define E_Y (EMIT_Y - SIGNAL_H / 2)
static const SignalSpawn schedule[] = {
    {0.40f, L_X, E_Y,  240.0f},
    {0.40f, R_X, E_Y, -240.0f},
    {1.30f, L_X, E_Y,  240.0f},
    {1.30f, R_X, E_Y, -240.0f},
    {2.20f, L_X, E_Y,  240.0f},
    {2.20f, R_X, E_Y, -240.0f},
    {3.10f, L_X, E_Y,  240.0f},
    {3.10f, R_X, E_Y, -240.0f},
};
#define SCHEDULE_LEN (int)(sizeof(schedule) / sizeof(schedule[0]))

void signal_start(void) {
    pattern_time = 0.0f;
    next_spawn = 0;
    for (int i = 0; i < MAX_SIGNAL; i++) {
        signals[i].active = false;
    }
}

static void spawn_signal(float x, float y, float vx) {
    for (int i = 0; i < MAX_SIGNAL; i++) {
        if (!signals[i].active) {
            signals[i].x = x;
            signals[i].y = y;
            signals[i].vx = vx;
            signals[i].vy = 0.0f;
            signals[i].age = 0.0f;
            signals[i].active = true;
            return;
        }
    }
}

static bool hit_player(float x, float y) {
    return !(x + SIGNAL_W < player.x || x > player.x + PLAYER_W ||
             y + SIGNAL_H < player.y || y > player.y + PLAYER_H);
}

void signal_update(float dt) {
    pattern_time += dt;

    while (next_spawn < SCHEDULE_LEN &&
           schedule[next_spawn].spawn_time <= pattern_time) {
        spawn_signal(schedule[next_spawn].x, schedule[next_spawn].y,
                     schedule[next_spawn].vx);
        next_spawn++;
    }

    for (int i = 0; i < MAX_SIGNAL; i++) {
        if (!signals[i].active) continue;

        signals[i].age += dt;

        // 스폰 직후 HOMING_TIME 동안만 매 프레임 플레이어 쪽으로 속도 재조준,
        // 이후엔 마지막 방향으로 직진
        if (signals[i].age < HOMING_TIME) {
            float cx = signals[i].x + SIGNAL_W / 2.0f;
            float cy = signals[i].y + SIGNAL_H / 2.0f;
            float dx = player.x + PLAYER_W / 2.0f - cx;
            float dy = player.y + PLAYER_H / 2.0f - cy;
            float d = sqrtf(dx * dx + dy * dy);
            if (d > 0.0001f) {
                signals[i].vx = dx / d * SIGNAL_SPEED;
                signals[i].vy = dy / d * SIGNAL_SPEED;
            }
        }

        signals[i].x += signals[i].vx * dt;
        signals[i].y += signals[i].vy * dt;

        if (hit_player(signals[i].x, signals[i].y)) {
            player_damage();
        }

        if (signals[i].x < -120 || signals[i].x > WINDOW_W + 120 ||
            signals[i].y < -120 || signals[i].y > WINDOW_H + 120) {
            signals[i].active = false;
        }
    }
}

bool signal_finished(void) {
    if (next_spawn < SCHEDULE_LEN) return false;
    for (int i = 0; i < MAX_SIGNAL; i++) {
        if (signals[i].active) return false;
    }
    return true;
}

// 전파 막대 하나: 노란 막대 + 전파 느낌의 선
static void draw_signal_bar(SDL_Renderer *r, SDL_Rect rect) {
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

// 좌우 송신소(기지국) 모양의 장식 오브젝트 하나를 그린다.
static void draw_station(SDL_Renderer *r, int x, int is_right_side) {
    int top = FLOOR_Y - STATION_BODY_H;   // 몸체 윗면 y

    SDL_SetRenderDrawColor(r, 95, 105, 120, 255);
    SDL_Rect body = { x, top, STATION_W, STATION_BODY_H };
    SDL_RenderFillRect(r, &body);

    SDL_SetRenderDrawColor(r, 210, 220, 235, 255);
    SDL_RenderDrawRect(r, &body);

    int mast_x = x + STATION_W / 2;
    int emit_y = top - STATION_MAST_H;    // 안테나 기둥 끝 = 전파 송출점
    SDL_RenderDrawLine(r, mast_x, emit_y, mast_x, top);
    SDL_RenderDrawLine(r, x + 18, top + 20, x + STATION_W - 18, top + 20);
    SDL_RenderDrawLine(r, x + 12, FLOOR_Y - 20, x + STATION_W - 12, FLOOR_Y - 20);

    int wave_dir = is_right_side ? -1 : 1;   // 플레이 영역(안쪽)을 향해 전파 송출

    SDL_SetRenderDrawColor(r, 255, 220, 80, 255);
    for (int i = 0; i < 3; i++) {
        int radius = 30 + i * 26;
        SDL_RenderDrawLine(r, mast_x, emit_y,
                           mast_x + wave_dir * radius, emit_y - radius / 2);
        SDL_RenderDrawLine(r, mast_x, emit_y,
                           mast_x + wave_dir * radius, emit_y + radius / 2);
    }
}

void signal_draw(SDL_Renderer *r) {
    draw_station(r, STATION_LEFT_X, 0);
    draw_station(r, STATION_RIGHT_X, 1);

    for (int i = 0; i < MAX_SIGNAL; i++) {
        if (!signals[i].active) continue;
        SDL_Rect rect = { (int)signals[i].x, (int)signals[i].y, SIGNAL_W, SIGNAL_H };
        draw_signal_bar(r, rect);
    }
}

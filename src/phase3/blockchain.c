#include "blockchain.h"
#include "../player.h"
#include <math.h>

#define MAX_BITCOINS 32
#define FALL_GRAVITY 300.0f // 비트코인 가속도 (px/s^2) — 클수록 빨리 떨어짐

// 화면에 실제로 떠 있는 비트코인 한 개의 상태
typedef struct {
    float x, y;
    float vy;
    int radius;
    bool active;
} Bitcoin;

// 미리 짜둔 스폰 스케줄 한 줄 (언제, 어디서, 얼마나 크게, 처음 속도)
typedef struct {
    float spawn_time;
    float x;
    int radius;
    float vy0;
} BitcoinSpawn;

static Bitcoin bitcoins[MAX_BITCOINS];
static float pattern_time = 0.0f;
static int next_spawn = 0;

// 패턴 데이터: 16개, 약 7초
static const BitcoinSpawn schedule[] = {
    {0.30f, 200, 100, 220.0f }, // 0.3초에 x=200 위치, 반지름 26, 초기 낙하 속도 220
    {0.80f, 600, 130, 180.0f },
    {1.20f, 900, 120, 260.0f },
    {1.70f, 400, 210, 150.0f },
    {2.10f, 1100, 170, 240.0f },
    {2.50f, 300, 290, 200.0f },
    {3.00f, 750, 90, 140.0f },
    {3.40f, 150, 150, 280.0f },
    {3.80f, 1000, 270, 200.0f },
    {4.20f, 500, 190, 240.0f },
    {4.60f, 850, 200, 170.0f },
    {5.00f, 250, 160, 220.0f },
    {5.40f, 650, 110, 260.0f },
    {5.80f, 1050, 240, 160.0f },
    {6.20f, 400, 300, 280.0f },
    {6.70f, 900, 150, 200.0f },
};
#define SCHEDULE_LEN (int)(sizeof(schedule) / sizeof(schedule[0]))

// 패턴 진입 시 한 번 호출 — 상태 초기화
void blockchain_start(void) {
    pattern_time = 0.0f;
    next_spawn = 0;
    for (int i = 0; i < MAX_BITCOINS; i++) {
        bitcoins[i].active = false;
    }
}

// 풀에서 빈 슬롯 하나 찾아 비트코인 활성화
static void spawn_bitcoin(float x, int radius, float vy0) {
    for (int i = 0; i < MAX_BITCOINS; i++) {
        if (!bitcoins[i].active) {            // 비어있는 첫 슬롯 발견
            bitcoins[i].x = x;
            bitcoins[i].y = -(float)radius - 10.0f; // 화면 위쪽에서 자연스럽게 떨어지게
            bitcoins[i].vy = vy0;
            bitcoins[i].radius = radius;
            bitcoins[i].active = true;
            return;
        }
    }
}

// 원(cx,cy,r)과 플레이어 충돌 — 가장 가까운 모서리 점 거리 비교
static bool circle_vs_player(float cx, float cy, float r) {
    float px = player.x;
    float py = player.y;

    // 원 중심을 플레이어 사각 안쪽으로 클램프 → 사각형에서 원에 가장 가까운 점
    float nx = cx;
    if (nx < px) nx = px;
    if (nx > px + PLAYER_W) nx = px + PLAYER_W;
    float ny = cy;
    if (ny < py) ny = py;
    if (ny > py + PLAYER_H) ny = py + PLAYER_H;

    float dx = cx - nx;
    float dy = cy - ny;
    return (dx * dx + dy * dy) <= (r * r); // 거리^2 ≤ 반지름^2 면 충돌
}

// 매 프레임 호출되는 메인 업데이트 — dt는 직전 프레임 경과 시간(초)
void blockchain_update(float dt) {
    pattern_time += dt;

    // 예약된 스폰 중 시간이 도래한 것들을 한 번에 모두 활성화
    while (next_spawn < SCHEDULE_LEN &&
           schedule[next_spawn].spawn_time <= pattern_time) {
        spawn_bitcoin(schedule[next_spawn].x,
                      schedule[next_spawn].radius,
                      schedule[next_spawn].vy0);
        next_spawn++;
    }

    // 활성 비트코인 전체에 대해 물리 + 충돌 + 화면 이탈 처리
    for (int i = 0; i < MAX_BITCOINS; i++) {
        if (!bitcoins[i].active) continue;

        bitcoins[i].vy += FALL_GRAVITY * dt;   // 중력으로 속도 증가
        bitcoins[i].y  += bitcoins[i].vy * dt; // 속도로 위치 갱신

        // 플레이어와 충돌 시 데미지 (무적 타이머는 player_damage 내부에서 처리됨)
        if (circle_vs_player(bitcoins[i].x, bitcoins[i].y,
                             (float)bitcoins[i].radius)) {
            player_damage();
        }

        // 화면 바닥보다 충분히 아래로 빠지면 슬롯 회수 (+40은 여유)
        if (bitcoins[i].y - bitcoins[i].radius > WINDOW_H + 40) {
            bitcoins[i].active = false;
        }
    }
}

// 패턴 종료 판정
bool blockchain_finished(void) {
    if (next_spawn < SCHEDULE_LEN) return false;
    for (int i = 0; i < MAX_BITCOINS; i++) {
        if (bitcoins[i].active) return false;
    }
    return true;
}

// 수평 스캔라인으로 원 채움
static void fill_circle(SDL_Renderer *r, int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)sqrtf((float)(radius * radius - dy * dy));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

// 비트코인 모양 ₿ 그리기
static void draw_bitcoin_B(SDL_Renderer *r, int cx, int cy, int radius) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);   // 글자색 검정

    int h = (int)(radius * 1.05f);
    int w = (int)(radius * 0.75f);
    int t = h / 5;
    if (t < 2) t = 2;

    int x = cx - w / 2;
    int y = cy - h / 2;

    // 왼쪽 세로 기둥(spine)과 가로 3획 (위/가운데/아래)
    SDL_Rect spine     = { x, y, t, h };
    SDL_Rect top_bar   = { x, y, w, t };
    SDL_Rect mid_bar   = { x, y + (h - t) / 2, w, t };
    SDL_Rect bot_bar   = { x, y + h - t, w, t };

    // 오른쪽 짧은 두 막대 (상/하) — ₿에서 D 모양 곡선을 대체하는 수직 획
    int rt_h = (h - t) / 2 - t;
    if (rt_h < 1) rt_h = 1;
    SDL_Rect right_top = { x + w - t, y + t, t, rt_h };
    SDL_Rect right_bot = { x + w - t, y + (h + t) / 2, t, rt_h };

    // 가로획 6개 모두 채우기
    SDL_RenderFillRect(r, &spine);
    SDL_RenderFillRect(r, &top_bar);
    SDL_RenderFillRect(r, &mid_bar);
    SDL_RenderFillRect(r, &bot_bar);
    SDL_RenderFillRect(r, &right_top);
    SDL_RenderFillRect(r, &right_bot);

    // ₿ 특유의 위아래 수직 돌출 (spine 위쪽 끝과 아래쪽 끝)
    int stub = h / 5;
    if (stub < 2) stub = 2;
    SDL_Rect stub_top = { x, y - stub, t, stub };
    SDL_Rect stub_bot = { x, y + h, t, stub };
    SDL_RenderFillRect(r, &stub_top);
    SDL_RenderFillRect(r, &stub_bot);
}

// 매 프레임 호출 — 활성 비트코인 모두 그림
void blockchain_draw(SDL_Renderer *r) {
    for (int i = 0; i < MAX_BITCOINS; i++) {
        if (!bitcoins[i].active) continue;
        int cx  = (int)bitcoins[i].x;
        int cy  = (int)bitcoins[i].y;
        int rad = bitcoins[i].radius;

        SDL_SetRenderDrawColor(r, 247, 197, 36, 255);   // 비트코인 황금색
        fill_circle(r, cx, cy, rad);

        draw_bitcoin_B(r, cx, cy, rad);
    }
}

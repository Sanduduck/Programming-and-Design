// 기초전자 패턴: 피카츄 등장 → 맵 중앙 이동 → 전기 번개 방사.
// 번개는 중력 없이 직선 등속으로 방사. 피카츄 본체(원) 또는 번개에 닿으면 데미지.

#include "elec.h"
#include "../player.h"
#include <math.h>

#define SHOW_X         220.0f
#define SHOW_Y         230.0f
#define CENTER_X       640.0f
#define CENTER_Y       330.0f

#define SHOW_TIME      1.5f
#define MOVE_TIME      1.5f

#define PIKA_R         46.0f
#define SPARK_R        28.0f

#define MAX_SPARKS     128

typedef enum { ST_SHOW = 0, ST_MOVE, ST_ATTACK, ST_DONE } Stage;

typedef struct { float x, y, vx, vy; bool active; } Spark;

// 번개 웨이브: t초에 count개를 base_ang부터 균등 각도로 speed 속도로 방사.
typedef struct { float t; int count; float base_ang; float speed; } Wave;

static const Wave WAVES[] = {
    { 0.40f, 10, 0.00f, 300.0f },
    { 1.50f, 10, 0.31f, 320.0f },
    { 2.60f, 12, 0.16f, 295.0f },
    { 3.70f, 12, 0.47f, 330.0f },
    { 4.80f, 12, 0.00f, 305.0f },
    { 5.90f, 14, 0.22f, 315.0f },
    { 7.00f, 14, 0.39f, 335.0f },
    { 8.10f, 16, 0.10f, 320.0f },
};
#define NUM_WAVES ((int)(sizeof(WAVES) / sizeof(WAVES[0])))

static Stage stage;
static float stage_t;
static float pika_x, pika_y;
static int   next_wave;
static Spark sparks[MAX_SPARKS];

void elec_start(void) {
    stage = ST_SHOW;
    stage_t = 0.0f;
    pika_x = SHOW_X;  pika_y = SHOW_Y;
    next_wave = 0;
    for (int i = 0; i < MAX_SPARKS; i++) sparks[i].active = false;
}

bool elec_finished(void) {
    return stage == ST_DONE;
}

// 원(cx,cy,r)이 플레이어 사각과 겹치는지 — 가장 가까운 모서리 점 거리 비교.
static bool hit_player(float cx, float cy, float r) {
    float nx = cx, ny = cy;
    if (nx < player.x)            nx = player.x;
    if (nx > player.x + PLAYER_W) nx = player.x + PLAYER_W;
    if (ny < player.y)            ny = player.y;
    if (ny > player.y + PLAYER_H) ny = player.y + PLAYER_H;
    float dx = cx - nx, dy = cy - ny;
    return dx * dx + dy * dy <= r * r;
}

static void spawn_wave(const Wave *wv) {
    for (int k = 0; k < wv->count; k++) {
        float ang = wv->base_ang + k * (6.2831f / wv->count);
        for (int i = 0; i < MAX_SPARKS; i++) {
            if (sparks[i].active) continue;
            sparks[i].x = pika_x;
            sparks[i].y = pika_y;
            sparks[i].vx = cosf(ang) * wv->speed;
            sparks[i].vy = sinf(ang) * wv->speed;
            sparks[i].active = true;
            break;
        }
    }
}

void elec_update(float dt) {
    if (stage == ST_DONE) return;
    stage_t += dt;

    if (stage == ST_SHOW) {
        if (stage_t >= SHOW_TIME) { stage = ST_MOVE; stage_t = 0.0f; }
        return;
    }

    if (stage == ST_MOVE) {
        float t = stage_t / MOVE_TIME;
        if (t > 1.0f) t = 1.0f;
        pika_x = SHOW_X + (CENTER_X - SHOW_X) * t;
        pika_y = SHOW_Y + (CENTER_Y - SHOW_Y) * t;
        if (hit_player(pika_x, pika_y, PIKA_R)) player_damage();
        if (stage_t >= MOVE_TIME) {
            pika_x = CENTER_X;  pika_y = CENTER_Y;
            stage = ST_ATTACK;  stage_t = 0.0f;
        }
        return;
    }

    // ST_ATTACK
    if (hit_player(pika_x, pika_y, PIKA_R)) player_damage();

    while (next_wave < NUM_WAVES && WAVES[next_wave].t <= stage_t) {
        spawn_wave(&WAVES[next_wave]);
        next_wave++;
    }

    bool any = false;
    for (int i = 0; i < MAX_SPARKS; i++) {
        Spark *s = &sparks[i];
        if (!s->active) continue;
        s->x += s->vx * dt;
        s->y += s->vy * dt;
        if (s->y > WINDOW_H + 60 || s->y < -60 || s->x < -60 || s->x > WINDOW_W + 60) {
            s->active = false;
            continue;
        }
        if (hit_player(s->x, s->y, SPARK_R)) player_damage();
        any = true;
    }

    if (next_wave >= NUM_WAVES && !any) stage = ST_DONE;
}

// ───── Drawing ─────

// 타원 호: a0~a1도 구간을 선분으로 이어 그림 (0=우, 90=하, 180=좌, 270=상).
static void draw_arc(SDL_Renderer *r, int cx, int cy, int rx, int ry, int a0, int a1) {
    int px = 0, py = 0;
    for (int a = a0; a <= a1; a += 6) {
        float rad = a * 3.14159265f / 180.0f;
        int x = cx + (int)(rx * cosf(rad));
        int y = cy + (int)(ry * sinf(rad));
        if (a > a0) SDL_RenderDrawLine(r, px, py, x, y);
        px = x; py = y;
    }
}

static void draw_ellipse(SDL_Renderer *r, int cx, int cy, int rx, int ry) {
    draw_arc(r, cx, cy, rx, ry, 0, 360);
}

// 볼록/오목 다각형 채우기 — 스캔라인.
static void fill_polygon(SDL_Renderer *r, const int *px, const int *py, int n) {
    int ymin = py[0], ymax = py[0];
    for (int i = 1; i < n; i++) {
        if (py[i] < ymin) ymin = py[i];
        if (py[i] > ymax) ymax = py[i];
    }
    for (int y = ymin; y <= ymax; y++) {
        int xs[12], cnt = 0;
        for (int i = 0; i < n; i++) {
            int j = (i + 1) % n;
            int y0 = py[i], y1 = py[j];
            if (y0 == y1) continue;
            if ((y >= y0) != (y >= y1)) {
                int x = px[i] + (px[j] - px[i]) * (y - y0) / (y1 - y0);
                if (cnt < 12) xs[cnt++] = x;
            }
        }
        for (int a = 0; a < cnt - 1; a++)
            for (int b = a + 1; b < cnt; b++)
                if (xs[b] < xs[a]) { int t = xs[a]; xs[a] = xs[b]; xs[b] = t; }
        for (int a = 0; a + 1 < cnt; a += 2)
            SDL_RenderDrawLine(r, xs[a], y, xs[a + 1], y);
    }
}

// 귀: 양 옆선(밑변은 머리에 묻혀 생략) + 끝 경계선(위 38%).
static void draw_ear(SDL_Renderer *r, int ax, int ay, int bl, int br, int by) {
    SDL_RenderDrawLine(r, ax, ay, bl, by);
    SDL_RenderDrawLine(r, ax, ay, br, by);
    float t = 0.38f;
    int ty = (int)(ay + (by - ay) * t);
    int tl = (int)(ax + (bl - ax) * t);
    int tr = (int)(ax + (br - ax) * t);
    SDL_RenderDrawLine(r, tl, ty, tr, ty);
}

// 꼬리 — 우측 위로 솟은 번개꼴 와이어프레임. (cx,cy)는 머리 중심.
// 외곽선 6점: 위 꼭지 → 우측 돌출 → 우측 안쪽 → 아래 꼭지 → 좌측 돌출 → 좌측 안쪽.
static void draw_tail(SDL_Renderer *r, int cx, int cy) {
    int tx[6] = { cx + 104, cx + 120, cx + 86, cx + 38, cx + 22, cx + 56 };
    int ty[6] = { cy - 58,  cy + 4,   cy + 4,  cy + 96, cy + 34, cy + 34 };
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    for (int i = 0; i < 6; i++) {
        int j = (i + 1) % 6;
        SDL_RenderDrawLine(r, tx[i], ty[i], tx[j], ty[j]);
    }
}

// 앉은 피카츄 — 흰색 와이어프레임. (cx,cy)는 머리 중심.
static void draw_pikachu(SDL_Renderer *r, int cx, int cy) {
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);

    // 꼬리 (몸통 뒤)
    draw_tail(r, cx, cy);

    // 몸통 (아래쪽 호 — 머리 안에서 시작해 아래로 불룩)
    draw_arc(r, cx, cy + 70, 66, 66, -54, 234);

    // 발
    draw_ellipse(r, cx - 32, cy + 138, 30, 15);
    draw_ellipse(r, cx + 32, cy + 138, 30, 15);

    // 팔
    draw_ellipse(r, cx - 30, cy + 78, 16, 19);
    draw_ellipse(r, cx + 30, cy + 78, 16, 19);

    // 머리
    draw_ellipse(r, cx, cy, 58, 54);

    // 귀
    draw_ear(r, cx - 56, cy - 126, cx - 42, cx - 12, cy - 42);
    draw_ear(r, cx + 56, cy - 126, cx + 12, cx + 42, cy - 42);

    // 눈 + 하이라이트
    draw_ellipse(r, cx - 24, cy - 6, 11, 13);
    draw_ellipse(r, cx + 24, cy - 6, 11, 13);
    draw_ellipse(r, cx - 27, cy - 10, 4, 4);
    draw_ellipse(r, cx + 21, cy - 10, 4, 4);

    // 볼
    draw_ellipse(r, cx - 42, cy + 16, 14, 12);
    draw_ellipse(r, cx + 42, cy + 16, 14, 12);

    // 코 + 입
    draw_ellipse(r, cx, cy + 2, 3, 2);
    SDL_RenderDrawLine(r, cx, cy + 6, cx - 11, cy + 16);
    SDL_RenderDrawLine(r, cx, cy + 6, cx + 11, cy + 16);
}

// 전기 번개: 노랑 채움 (6각 번개 모양).
static void draw_spark(SDL_Renderer *r, int x, int y) {
    int px[6] = { x - 8, x + 18, x + 2, x + 13, x - 18, x - 2 };
    int py[6] = { y - 38, y - 8,  y - 8, y + 38, y + 8,  y + 8 };
    SDL_SetRenderDrawColor(r, 250, 220, 40, 255);
    fill_polygon(r, px, py, 6);
}

void elec_draw(SDL_Renderer *r) {
    if (stage == ST_DONE) return;

    draw_pikachu(r, (int)pika_x, (int)pika_y);
    for (int i = 0; i < MAX_SPARKS; i++) {
        if (sparks[i].active) draw_spark(r, (int)sparks[i].x, (int)sparks[i].y);
    }
}

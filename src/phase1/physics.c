// 대학물리학 패턴: 오실로스코프 인트로 → 줌인 → 파형 회피

#include "physics.h"
#include "../player.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WINDOW_W      1280
#define WINDOW_H      720

#define INTRO_TIME    2.0f
#define ZOOM_TIME     1.6f
#define AVOID_TIME    10.0f

#define WAVE_CY       240.0f
#define WAVE_AMP      220.0f
#define WAVE_FREQ     2.5f
#define WAVE_THICK    3

#define HLINE_CYCLE   4.0f
#define HLINE_BLINK   1.0f
#define HLINE_ATTACK  0.5f
#define HLINE_THICK   4

typedef enum { PHYS_INTRO, PHYS_ZOOM, PHYS_AVOID, PHYS_DONE } PhysicsStage;

static const SDL_Rect INTRO_RECT = { 320, 210, 640, 300 };
static const int hline_y[2] = { 540, 590 };

static PhysicsStage stage;
static float stage_time, wave_phase, total_time;

static float clamp01(float t) { return t < 0 ? 0 : (t > 1 ? 1 : t); }
static float lerp_f(float a, float b, float t) { return a + (b - a) * clamp01(t); }

static SDL_Rect screen_rect(void) {
    if (stage == PHYS_INTRO) return INTRO_RECT;
    if (stage != PHYS_ZOOM) {
        SDL_Rect full = { 0, 0, WINDOW_W, WINDOW_H };
        return full;
    }
    float u = 1.0f - clamp01(stage_time / ZOOM_TIME);
    float t = 1.0f - u * u;
    SDL_Rect r = {
        (int)lerp_f((float)INTRO_RECT.x, 0.0f, t),
        (int)lerp_f((float)INTRO_RECT.y, 0.0f, t),
        (int)lerp_f((float)INTRO_RECT.w, (float)WINDOW_W, t),
        (int)lerp_f((float)INTRO_RECT.h, (float)WINDOW_H, t),
    };
    return r;
}

static float wave_y_at(float x) {
    float angle = x / (float)WINDOW_W * WAVE_FREQ * 2.0f * (float)M_PI + wave_phase;
    return WAVE_CY + WAVE_AMP * sinf(angle);
}

// 라인 사이클 상태: 0=idle, 1=blink, 2=attack. 라인2는 cycle/2 오프셋.
static int hline_phase(int idx) {
    float t = fmodf(total_time + (idx ? HLINE_CYCLE * 0.5f : 0.0f), HLINE_CYCLE);
    if (t >= HLINE_CYCLE - HLINE_ATTACK) return 2;
    if (t >= HLINE_CYCLE - HLINE_ATTACK - HLINE_BLINK) return 1;
    return 0;
}

void physics_start(void) {
    stage = PHYS_INTRO;
    stage_time = wave_phase = total_time = 0.0f;
}

void physics_update(float dt) {
    stage_time += dt;
    total_time += dt;

    float scroll = 1.2f;
    if (stage == PHYS_ZOOM)  scroll = lerp_f(1.2f, 3.0f, stage_time / ZOOM_TIME);
    if (stage == PHYS_AVOID) scroll = lerp_f(3.0f, 4.5f, stage_time / AVOID_TIME);
    wave_phase += scroll * dt;

    if (stage == PHYS_INTRO && stage_time >= INTRO_TIME) { stage = PHYS_ZOOM;  stage_time = 0; }
    else if (stage == PHYS_ZOOM && stage_time >= ZOOM_TIME) { stage = PHYS_AVOID; stage_time = 0; }
    else if (stage == PHYS_AVOID) {
        float py = player.y, pb = player.y + PLAYER_H;

        for (float sx = player.x; sx <= player.x + 50.0f; sx += 4.0f) {
            float wy = wave_y_at(sx);
            if (wy + WAVE_THICK >= py && wy - WAVE_THICK <= pb) { player_damage(); break; }
        }
        for (int i = 0; i < 2; i++) {
            if (hline_phase(i) != 2) continue;
            float ly = (float)hline_y[i];
            if (ly + HLINE_THICK >= py && ly - HLINE_THICK <= pb) { player_damage(); break; }
        }

        if (stage_time >= AVOID_TIME) stage = PHYS_DONE;
    }
}

bool physics_finished(void) { return stage == PHYS_DONE; }

static int map_x(SDL_Rect sr, float lx) { return sr.x + (int)(lx / (float)WINDOW_W * (float)sr.w); }
static int map_y(SDL_Rect sr, float ly) { return sr.y + (int)(ly / (float)WINDOW_H * (float)sr.h); }

static void draw_bg(SDL_Renderer *r, SDL_Rect sr) {
    SDL_SetRenderDrawColor(r, 5, 18, 28, 255);
    SDL_RenderFillRect(r, &sr);
    SDL_SetRenderDrawColor(r, 0, 80, 100, 255);
    for (int i = 1; i < 10; i++) {
        int gx = sr.x + sr.w * i / 10;
        SDL_RenderDrawLine(r, gx, sr.y, gx, sr.y + sr.h);
    }
    for (int i = 1; i < 6; i++) {
        int gy = sr.y + sr.h * i / 6;
        SDL_RenderDrawLine(r, sr.x, gy, sr.x + sr.w, gy);
    }
}

static void draw_wave(SDL_Renderer *r, SDL_Rect sr) {
    SDL_SetRenderDrawColor(r, 90, 230, 180, 255);
    int prev_x = map_x(sr, 0.0f), prev_y = map_y(sr, wave_y_at(0.0f));
    for (int i = 1; i <= 256; i++) {
        float lx = (float)i * (float)WINDOW_W / 256.0f;
        int sx = map_x(sr, lx), sy = map_y(sr, wave_y_at(lx));
        for (int dy = -WAVE_THICK; dy <= WAVE_THICK; dy++)
            SDL_RenderDrawLine(r, prev_x, prev_y + dy, sx, sy + dy);
        prev_x = sx; prev_y = sy;
    }
}

static void draw_hlines(SDL_Renderer *r, SDL_Rect sr) {
    static const Uint8 COLORS[3][3] = {
        {  90,  30,  30 },  // idle
        { 255, 140,  50 },  // blink (on)
        { 255,  60,  60 },  // attack
    };
    static const int HALF[3] = { HLINE_THICK - 2, HLINE_THICK, HLINE_THICK + 2 };

    for (int i = 0; i < 2; i++) {
        int p = hline_phase(i);
        const Uint8 *c = COLORS[p];
        Uint8 dim[3] = { 90, 40, 30 };
        if (p == 1 && (((int)(total_time * 10.0f) & 1) != 0)) c = dim;

        int cy = map_y(sr, (float)hline_y[i]);
        SDL_SetRenderDrawColor(r, c[0], c[1], c[2], 255);
        SDL_Rect rect = { sr.x, cy - HALF[p], sr.w, HALF[p] * 2 + 1 };
        SDL_RenderFillRect(r, &rect);
    }
}

static void fill_disc(SDL_Renderer *r, int cx, int cy, int rad) {
    for (int dy = -rad; dy <= rad; dy++) {
        int dx = (int)sqrtf((float)(rad * rad - dy * dy));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

static void draw_frame(SDL_Renderer *r, SDL_Rect sr) {
    SDL_Rect outer = { sr.x - 30, sr.y - 30, sr.w + 60, sr.h + 130 };
    SDL_SetRenderDrawColor(r, 25, 30, 35, 255);
    SDL_RenderFillRect(r, &outer);
    SDL_SetRenderDrawColor(r, 0, 200, 220, 255);
    SDL_RenderDrawRect(r, &outer);

    SDL_Rect bezel = { sr.x - 6, sr.y - 6, sr.w + 12, sr.h + 12 };
    SDL_SetRenderDrawColor(r, 0, 150, 180, 255);
    SDL_RenderDrawRect(r, &bezel);

    int knob_y = sr.y + sr.h + 50;
    SDL_SetRenderDrawColor(r, 180, 180, 180, 255);
    for (int i = 0; i < 3; i++) fill_disc(r, sr.x + sr.w / 2 + (i - 1) * 95, knob_y, 22);
}

void physics_draw(SDL_Renderer *r) {
    SDL_Rect sr = screen_rect();
    if (stage == PHYS_INTRO || stage == PHYS_ZOOM) draw_frame(r, sr);
    draw_bg(r, sr);
    SDL_RenderSetClipRect(r, &sr);
    draw_wave(r, sr);
    draw_hlines(r, sr);
    SDL_RenderSetClipRect(r, NULL);
}

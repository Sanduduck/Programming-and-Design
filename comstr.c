// 컴퓨터구조 패턴: 부품을 보여주고(말풍선) → 던지고 → 파편으로 깨짐.
// CPU, 본체, 메인보드 3종이 순차적으로 등장. 모두 흰색 와이어프레임 + 흰색 파편.

#include "comstr.h"
#include "../player.h"
#include <SDL_ttf.h>
#include <math.h>

#define W             1280
#define H             720
#define COMSTR_PLAYER_W      50
#define COMSTR_PLAYER_H      50

#define SHOW_X        200.0f
#define SHOW_Y        180.0f
#define TARGET_X      640.0f
#define TARGET_Y      550.0f

#define SHOW_TIME     1.8f
#define THROW_TIME    1.0f
#define BROKEN_TIME   2.0f
#define GRAVITY       700.0f
#define FRAG_GRAVITY  300.0f

#define MAX_FRAGS     12

typedef enum { PART_CPU = 0, PART_CASE, PART_BOARD, N_PARTS } PartKind;
typedef enum { STAGE_SHOWING = 0, STAGE_THROWING, STAGE_BROKEN, STAGE_DONE } Stage;

typedef struct { float x, y, vx, vy; int size; } Fragment;
typedef struct { int dx, dy, size; float speed; } FragSpec;

static const char *part_names[N_PARTS] = { "CPU", "본체", "메인보드" };
static const int   part_w[N_PARTS]     = { 110, 140, 240 };
static const int   part_h[N_PARTS]     = { 110, 200, 160 };

// 12개 파편 고정 데이터. 각도는 i * 30° 균등 분배, 나머지는 시각 다양성을 위한 손튜닝.
static const FragSpec FRAG_SPECS[MAX_FRAGS] = {
    { -20, -12, 50, 520.0f }, {  18,  -8, 58, 460.0f },
    { -10,  14, 44, 580.0f }, {  22,   6, 64, 440.0f },
    { -22, -14, 48, 540.0f }, {  12,  12, 54, 500.0f },
    { -16, -10, 60, 560.0f }, {  24,  -4, 44, 480.0f },
    {  -8,  10, 68, 600.0f }, {  16, -14, 50, 420.0f },
    { -22,   4, 56, 520.0f }, {   8,   8, 48, 480.0f },
};

static int       current;
static Stage     stage;
static float     stage_t;
static float     part_x, part_y, part_vx, part_vy;
static Fragment  frags[MAX_FRAGS];
static TTF_Font *bubble_font = NULL;

static void reset_for_show(void) {
    stage = STAGE_SHOWING;
    stage_t = 0.0f;
    part_x = SHOW_X;  part_y = SHOW_Y;
    part_vx = part_vy = 0.0f;
}

void comstr_start(void) {
    current = 0;
    reset_for_show();
    if (!bubble_font) bubble_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 22);
}

bool comstr_finished(void) {
    return stage == STAGE_DONE;
}

static bool hit_player(float x, float y, int w, int h) {
    return !(x + w < player.x || x > player.x + COMSTR_PLAYER_W ||
             y + h < player.y || y > player.y + COMSTR_PLAYER_H);
}

void comstr_update(float dt) {
    stage_t += dt;

    if (stage == STAGE_SHOWING) {
        if (stage_t >= SHOW_TIME) {
            stage = STAGE_THROWING;
            stage_t = 0.0f;
            part_vx = (TARGET_X - part_x) / THROW_TIME;
            part_vy = ((TARGET_Y - part_y) - 0.5f * GRAVITY * THROW_TIME * THROW_TIME) / THROW_TIME;
        }
    } else if (stage == STAGE_THROWING) {
        part_vy += GRAVITY * dt;
        part_x  += part_vx * dt;
        part_y  += part_vy * dt;
        int pw = part_w[current], ph = part_h[current];
        if (hit_player(part_x - pw / 2.0f, part_y - ph / 2.0f, pw, ph)) player_damage();
        if (part_y >= TARGET_Y || stage_t >= THROW_TIME + 0.3f) {
            for (int i = 0; i < MAX_FRAGS; i++) {
                Fragment       *f = &frags[i];
                const FragSpec *s = &FRAG_SPECS[i];
                float ang = i * (6.2831f / MAX_FRAGS);
                f->x  = part_x + (float)s->dx;
                f->y  = part_y + (float)s->dy;
                f->vx = cosf(ang) * s->speed;
                f->vy = sinf(ang) * s->speed - 220.0f;
                f->size = s->size;
            }
            stage = STAGE_BROKEN;
            stage_t = 0.0f;
        }
    } else if (stage == STAGE_BROKEN) {
        for (int i = 0; i < MAX_FRAGS; i++) {
            Fragment *f = &frags[i];
            f->vy += FRAG_GRAVITY * dt;
            f->x  += f->vx * dt;
            f->y  += f->vy * dt;
            if (hit_player(f->x, f->y, f->size, f->size)) player_damage();
        }
        if (stage_t >= BROKEN_TIME) {
            current++;
            if (current >= N_PARTS) stage = STAGE_DONE;
            else                    reset_for_show();
        }
    }
}

// ───── Drawing ─────

static void set_color(SDL_Renderer *r, Uint8 cr, Uint8 cg, Uint8 cb) {
    SDL_SetRenderDrawColor(r, cr, cg, cb, 255);
}

static void fill_rect(SDL_Renderer *r, int x, int y, int w, int h) {
    SDL_Rect rc = { x, y, w, h };
    SDL_RenderFillRect(r, &rc);
}

static void draw_outline(SDL_Renderer *r, int x, int y, int w, int h) {
    SDL_Rect rc = { x, y, w, h };
    SDL_RenderDrawRect(r, &rc);
}

static void draw_circle(SDL_Renderer *r, int cx, int cy, int radius) {
    int x = radius, y = 0, err = 0;
    while (x >= y) {
        SDL_RenderDrawPoint(r, cx + x, cy + y);
        SDL_RenderDrawPoint(r, cx + y, cy + x);
        SDL_RenderDrawPoint(r, cx - y, cy + x);
        SDL_RenderDrawPoint(r, cx - x, cy + y);
        SDL_RenderDrawPoint(r, cx - x, cy - y);
        SDL_RenderDrawPoint(r, cx - y, cy - x);
        SDL_RenderDrawPoint(r, cx + y, cy - x);
        SDL_RenderDrawPoint(r, cx + x, cy - y);
        y++;
        if (err <= 0) err += 2*y + 1;
        if (err > 0)  { x--; err -= 2*x + 1; }
    }
}

static void draw_part(SDL_Renderer *r, int kind, int cx, int cy) {
    set_color(r, 255, 255, 255);
    int w = part_w[kind], h = part_h[kind];
    int bx = cx - w/2, by = cy - h/2;
    draw_outline(r, bx, by, w, h);
    if (kind == PART_CPU) {
        int dw = w * 6 / 10, dh = h * 6 / 10;
        draw_outline(r, cx - dw/2, cy - dh/2, dw, dh);
    } else if (kind == PART_CASE) {
        draw_circle(r, bx + w - 18, by + 18, 5);
        draw_outline(r, bx + 10, by + 36, w - 20, 8);
    } else if (kind == PART_BOARD) {
        int sw = w * 3 / 10;
        draw_outline(r, cx - sw/2, cy - sw/2, sw, sw);
        int slot_w = w / 3;
        draw_outline(r, bx + w - slot_w - 20, by + 16, slot_w, 8);
        draw_outline(r, bx + w - slot_w - 20, by + 30, slot_w, 8);
    }
}

static void draw_bubble(SDL_Renderer *r, int kind, int pcx, int pcy) {
    if (!bubble_font) return;
    char msg[64];
    SDL_snprintf(msg, sizeof(msg), "이거 너 컴퓨터 %s", part_names[kind]);
    int tw, th;
    TTF_SizeUTF8(bubble_font, msg, &tw, &th);

    int bw = tw + 28, bh = th + 16;
    int bx = pcx - bw / 2;
    int by = pcy - part_h[kind] / 2 - bh - 12;
    if (by < 6) by = 6;
    if (bx < 6) bx = 6;
    if (bx + bw > W - 6) bx = W - 6 - bw;

    set_color(r, 255, 255, 255);
    fill_rect(r, bx, by, bw, bh);
    set_color(r, 10, 10, 10);
    draw_outline(r, bx, by, bw, bh);

    SDL_Color col = { 15, 15, 20, 255 };
    SDL_Surface *surf = TTF_RenderUTF8_Blended(bubble_font, msg, col);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst = { bx + 14, by + 8, surf->w, surf->h };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void comstr_draw(SDL_Renderer *r) {
    if (stage == STAGE_SHOWING || stage == STAGE_THROWING) {
        draw_part(r, current, (int)part_x, (int)part_y);
        if (stage == STAGE_SHOWING) draw_bubble(r, current, (int)part_x, (int)part_y);
    } else if (stage == STAGE_BROKEN) {
        set_color(r, 255, 255, 255);
        for (int i = 0; i < MAX_FRAGS; i++) {
            fill_rect(r, (int)frags[i].x, (int)frags[i].y, frags[i].size, frags[i].size);
        }
    }
}

// 프로그래밍언어: C 계산기 코드 라인이 상승. 단어 사이 공백만 통과.
// 좌/우 끝 벽 + 토큰을 1방향 발판으로, 천장(y<0) 닿으면 데미지.
// 시작 1초간 화면 상단에 한글 경고 문구 표시.

#include "proglang.h"
#include "../player.h"
#include <SDL_ttf.h>
#include <string.h>

#define W            1280
#define H            720
#define PLAYER_W     50
#define PLAYER_H     50

#define MAX_LINES    20
#define MAX_BLOCKS   16
#define MARGIN       40
#define LINE_GAP     14
#define RISE_SPEED   100.0f

typedef struct { int x, w; bool wall; } Block;

typedef struct {
    const char *text;
    Block blocks[MAX_BLOCKS];
    int n_blocks;
    float y;
    bool active;
} CodeLine;

static const char *calc_code[] = {
    "#include <stdio.h>",
    "",
    "int main(void) {",
    "    double a, b;",
    "    char op;",
    "    scanf(\"%lf %c %lf\", &a, &op, &b);",
    "    double result = 0;",
    "    switch (op) {",
    "    case '+': result = a + b; break;",
    "    case '-': result = a - b; break;",
    "    case '*': result = a * b; break;",
    "    case '/': result = a / b; break;",
    "    }",
    "    printf(\"%lf\\n\", result);",
    "    return 0;",
    "}",
};
#define N_CODE (int)(sizeof(calc_code) / sizeof(*calc_code))

static CodeLine  lines[MAX_LINES];
static float     pattern_time, spawn_interval = 1.0f;
static int       next_spawn, line_height = 50;
static TTF_Font *code_font = NULL;
static TTF_Font *warn_font = NULL;

static int seg_w(const char *s, int len) {
    if (len <= 0 || !code_font) return 0;
    char buf[128];
    if (len > 127) len = 127;
    memcpy(buf, s, len); buf[len] = 0;
    int w, h;
    TTF_SizeUTF8(code_font, buf, &w, &h);
    return w;
}

static void load_font(void) {
    if (code_font) return;
    const char *path = "C:/Windows/Fonts/consola.ttf";
    TTF_Font *probe = TTF_OpenFont(path, 40);
    if (!probe) { path = "C:/Windows/Fonts/malgun.ttf"; probe = TTF_OpenFont(path, 40); }

    int size = 40;
    if (probe) {
        const char *longest = calc_code[0];
        for (int i = 1; i < N_CODE; i++)
            if (strlen(calc_code[i]) > strlen(longest)) longest = calc_code[i];
        int lw, lh;
        TTF_SizeUTF8(probe, longest, &lw, &lh);
        TTF_CloseFont(probe);
        if (lw > 0) size = 40 * (W - 2 * MARGIN) / lw;
        if (size < 20) size = 20;
        if (size > 96) size = 96;
    }
    code_font = TTF_OpenFont(path, size);
    if (code_font) line_height = TTF_FontHeight(code_font) + 6;
    spawn_interval = (line_height + LINE_GAP) / RISE_SPEED;

    if (!warn_font) warn_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 48);
}

static void add_block(CodeLine *l, int x, int w, bool wall) {
    if (l->n_blocks >= MAX_BLOCKS) return;
    Block *b = &l->blocks[l->n_blocks++];
    b->x = x; b->w = w; b->wall = wall;
}

// 단일 토큰 라인("}", "{")은 양옆 벽 없이 통과 가능.
static void build_line(CodeLine *l) {
    l->n_blocks = 0;
    int len = (int)strlen(l->text);
    if (len == 0 || !code_font) return;

    int tx[MAX_BLOCKS], tw[MAX_BLOCKS], n = 0, cursor = 0;
    for (int i = 0; i < len; ) {
        int ss = i;
        while (i < len && (l->text[i] == ' ' || l->text[i] == '\t')) i++;
        if (i > ss) cursor += seg_w(l->text + ss, i - ss);
        if (i >= len) break;
        int ts = i;
        while (i < len && l->text[i] != ' ' && l->text[i] != '\t') i++;
        int width = seg_w(l->text + ts, i - ts);
        if (n < MAX_BLOCKS) { tx[n] = MARGIN + cursor; tw[n] = width; n++; }
        cursor += width;
    }
    if (n == 0) return;

    bool walls = (n >= 2);
    if (walls) add_block(l, 0, tx[0], true);
    for (int k = 0; k < n; k++) add_block(l, tx[k], tw[k], false);
    if (walls) {
        int end = tx[n-1] + tw[n-1];
        if (end < W) add_block(l, end, W - end, true);
    }
}

void proglang_start(void) {
    pattern_time = 0;
    next_spawn = 0;
    for (int i = 0; i < MAX_LINES; i++) lines[i].active = false;
    load_font();
}

void proglang_update(float dt) {
    pattern_time += dt;

    while (next_spawn < N_CODE && next_spawn * spawn_interval <= pattern_time) {
        for (int i = 0; i < MAX_LINES; i++) {
            if (lines[i].active) continue;
            lines[i].text   = calc_code[next_spawn];
            lines[i].y      = (float)H + 10.0f;
            lines[i].active = true;
            build_line(&lines[i]);
            break;
        }
        next_spawn++;
    }

    for (int i = 0; i < MAX_LINES; i++) {
        if (!lines[i].active) continue;
        lines[i].y -= RISE_SPEED * dt;
        if (lines[i].y + line_height < 0) lines[i].active = false;
    }

    // 1방향 발판: 떨어지는 중 발 중심 아래 가장 높은 블록 표면으로 스냅.
    if (player.vy >= 0) {
        float feet = player.y + PLAYER_H;
        int   pcx  = (int)player.x + PLAYER_W / 2;
        float best = 0; bool found = false;
        for (int i = 0; i < MAX_LINES; i++) {
            if (!lines[i].active) continue;
            float top = lines[i].y;
            if (feet < top || feet > top + line_height) continue;
            if (found && top >= best) continue;
            for (int j = 0; j < lines[i].n_blocks; j++) {
                Block *b = &lines[i].blocks[j];
                if (pcx >= b->x && pcx < b->x + b->w) { best = top; found = true; break; }
            }
        }
        if (found) {
            player.y = best - PLAYER_H;
            player.vy = 0;
            player.on_ground = true;
            player.jump_count = 0;
        }
    }

    if (player.y < 0) {
        player.y = 0;
        if (player.vy < 0) player.vy = 0;
        player_damage();
    }
}

bool proglang_finished(void) {
    if (next_spawn < N_CODE) return false;
    for (int i = 0; i < MAX_LINES; i++) if (lines[i].active) return false;
    return true;
}

// 텍스트를 (left, top)에 흰색으로 렌더. 빈 문자열/폰트 없음이면 무시.
static void blit_text(SDL_Renderer *r, TTF_Font *f, const char *s, int left, int top) {
    if (!f || !s || !*s) return;
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Surface *surf = TTF_RenderUTF8_Blended(f, s, white);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst = { left, top, surf->w, surf->h };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void proglang_draw(SDL_Renderer *r) {
    if (!code_font) return;
    SDL_SetRenderDrawColor(r, 0, 80, 100, 255);

    int text_dy = (line_height - TTF_FontHeight(code_font)) / 2;
    for (int i = 0; i < MAX_LINES; i++) {
        if (!lines[i].active) continue;
        for (int j = 0; j < lines[i].n_blocks; j++) {
            Block *b = &lines[i].blocks[j];
            if (!b->wall) continue;
            SDL_Rect wr = { b->x, (int)lines[i].y, b->w, line_height };
            SDL_RenderFillRect(r, &wr);
        }
        blit_text(r, code_font, lines[i].text, MARGIN, (int)lines[i].y + text_dy);
    }

    if (pattern_time < 1.0f && warn_font) {
        int ww, wh;
        TTF_SizeUTF8(warn_font, "화면 위에 닿지마", &ww, &wh);
        blit_text(r, warn_font, "화면 위에 닿지마", (W - ww) / 2, 80);
    }
}

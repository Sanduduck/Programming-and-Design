// 대학수학 패턴 — 고정 3문제(답: -1, 0, +1 순). QUIZ → CHOICE → REVEAL.
// 정답 zone 인덱스 = quiz_index (questions 배열 인덱스와 동일).

#include "math.h"
#include "../player.h"
#include <SDL_ttf.h>
#include <stdio.h>

#define WINDOW_W    1280
#define WINDOW_H    720
#define FLOOR_Y     600

#define QUIZ_COUNT  3
#define DUR_QUIZ    3.0f
#define DUR_CHOICE  4.0f
#define DUR_REVEAL  1.5f

typedef enum { M_QUIZ, M_CHOICE, M_REVEAL, M_DONE } MathState;

static const char *questions[QUIZ_COUNT] = {
    "cos(pi)",   // = -1
    "ln(1)",     // =  0
    "e^0",       // = +1
};

static MathState  state;
static int        quiz_index;
static float      timer;
static TTF_Font  *font = NULL;

static int player_zone(void) {
    int z = (int)((player.x + PLAYER_W / 2) * 3 / WINDOW_W);
    return z < 0 ? 0 : (z > 2 ? 2 : z);
}

void math_start(void) {
    if (!font) font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 56);
    quiz_index = 0;
    state = M_QUIZ;
    timer = 0;
    player.x = (float)(WINDOW_W / 2 - PLAYER_W / 2);
}

void math_update(float dt) {
    timer += dt;
    if (state == M_QUIZ && timer >= DUR_QUIZ) {
        state = M_CHOICE; timer = 0;
    } else if (state == M_CHOICE && timer >= DUR_CHOICE) {
        if (player_zone() != quiz_index) player_damage();
        state = M_REVEAL; timer = 0;
    } else if (state == M_REVEAL && timer >= DUR_REVEAL) {
        quiz_index++;
        state = (quiz_index >= QUIZ_COUNT) ? M_DONE : M_QUIZ;
        timer = 0;
    }
}

bool math_finished(void) {
    return state == M_DONE;
}

static void draw_text(SDL_Renderer *r, const char *s, int cx, int cy, SDL_Color c) {
    if (!font || !s) return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, s, c);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect d = { cx - surf->w / 2, cy - surf->h / 2, surf->w, surf->h };
    SDL_RenderCopy(r, tex, NULL, &d);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void math_draw(SDL_Renderer *r) {
    SDL_Color white = { 255, 255, 255, 255 };
    char qbuf[64];
    snprintf(qbuf, sizeof(qbuf), "%s  답은?", questions[quiz_index]);

    if (state == M_QUIZ) {
        draw_text(r, qbuf, WINDOW_W / 2, WINDOW_H / 2, white);
        return;
    }

    static const Uint8 PALETTE[2][2][3] = {
        // [reveal][hit] = { r, g, b }
        { {  20,  30,  40 }, {   0,  80, 100 } },  // choice: off / highlighted
        { { 130,  30,  30 }, {  30, 140,  70 } },  // reveal: wrong / correct
    };
    int reveal = (state == M_REVEAL);
    int target = reveal ? quiz_index : player_zone();
    int zone_w = WINDOW_W / 3;

    for (int i = 0; i < 3; i++) {
        const Uint8 *c = PALETTE[reveal][i == target];
        SDL_SetRenderDrawColor(r, c[0], c[1], c[2], 255);
        SDL_Rect zone = { i * zone_w, 0, zone_w, FLOOR_Y };
        SDL_RenderFillRect(r, &zone);

        char buf[8];
        snprintf(buf, sizeof(buf), "%d", i - 1);
        draw_text(r, buf, i * zone_w + zone_w / 2, 250, white);
    }

    draw_text(r, qbuf, WINDOW_W / 2, 60, white);
}

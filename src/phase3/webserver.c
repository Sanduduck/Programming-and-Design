// 웹서버프로그래밍 패턴 (정복래) — HTTP 메서드 ↔ 의미 매칭
// 하단에 GET / POST / PUT / DELETE 텍스트 블록 4개 배치
// 상단 패널에 의미 설명이 한글로 표시되면, 플레이어가 해당 블록 위에서 ↓ 키로 머리 위로 들어올림
// 폭탄이 떨어지는 동안 올바른 블록을 들고 있으면 막힘, 아니면 데미지
// 4메서드를 셔플 순서로 모두 출제 → 총 4라운드

#include "webserver.h"
#include "../player.h"
#include <SDL_ttf.h>
#include <math.h>
#include <stdlib.h>

#define WINDOW_W       1280
#define FLOOR_Y        600

#define BLOCK_W        130
#define BLOCK_H        60
#define BLOCK_TOP_Y    (FLOOR_Y - BLOCK_H)

#define MEANING_BAR_W  420
#define MEANING_BAR_H  72
#define MEANING_BAR_Y  80

#define BOMB_R         44                  // 원래: 24 — 더 크게 표시

// 페이즈별 시간(초)
#define INTRO_TIME     1.8f
#define FALL_TIME      1.8f
#define RESOLVE_TIME   1.4f

typedef enum {
    METHOD_GET = 0,
    METHOD_POST,
    METHOD_PUT,
    METHOD_DELETE,
    METHOD_COUNT
} HttpMethod;

typedef enum {
    PHASE_INTRO = 0,    // 의미 표시 + 블록 선택 시간
    PHASE_FALLING,      // 폭탄 낙하 중
    PHASE_RESOLVE,      // 결과 잔상 표시
    PHASE_DONE          // 4라운드 종료
} RoundPhase;

// 메서드별 고정 x좌표 + 영문 토큰 + 한글 설명
static const struct {
    float x;
    const char *token;
    const char *meaning;
} METHOD_DATA[METHOD_COUNT] = {
    { 240.0f, "GET",    "데이터 조회"      },
    { 480.0f, "POST",   "데이터 생성"      },
    { 720.0f, "PUT",    "데이터 전체 수정" },
    { 960.0f, "DELETE", "데이터 삭제"      },
};

static float phase_time = 0.0f;
static RoundPhase phase = PHASE_INTRO;
static int round_idx = 0;
static int order[METHOD_COUNT];
static int holding = -1;
static bool prev_down = false;

static float bomb_y = 0.0f;
static float bomb_vy = 0.0f;

static bool  last_blocked = false;
static float resolve_cx = 0.0f;
static float resolve_cy = 0.0f;

static TTF_Font *token_font   = NULL;   // 블록 위 영문 메서드명
static TTF_Font *meaning_font = NULL;   // 상단 한글 설명

static void load_font(void) {
    if (!token_font)   token_font   = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 26);
    if (!meaning_font) meaning_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 38);
}

void webserver_start(void) {
    load_font();

    phase_time = 0.0f;
    phase = PHASE_INTRO;
    round_idx = 0;
    holding = -1;
    prev_down = false;

    for (int i = 0; i < METHOD_COUNT; i++) order[i] = i;
    // Fisher-Yates 셔플 — 매 플레이마다 출제 순서가 달라짐
    for (int i = METHOD_COUNT - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
    }
}

static int correct_method(void) {
    return order[round_idx];
}

static bool player_on_block(int i) {
    float bx = METHOD_DATA[i].x;
    return player.x + PLAYER_W > bx && player.x < bx + (float)BLOCK_W;
}

void webserver_update(float dt) {
    if (phase == PHASE_DONE) return;
    phase_time += dt;

    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    bool cur_down = keys[SDL_SCANCODE_DOWN] != 0;
    bool just_pressed = cur_down && !prev_down;
    prev_down = cur_down;

    if (just_pressed) {
        int on_block = -1;
        for (int i = 0; i < METHOD_COUNT; i++) {
            if (player_on_block(i)) { on_block = i; break; }
        }
        if (on_block >= 0) {
            holding = (holding == on_block) ? -1 : on_block;
        } else {
            holding = -1;
        }
    }

    if (phase == PHASE_FALLING) {
        bomb_y += bomb_vy * dt;
    }

    if (phase == PHASE_INTRO) {
        if (phase_time >= INTRO_TIME) {
            bomb_y = -(float)BOMB_R;
            float target_y = player.y - (float)BLOCK_H * 0.5f;
            bomb_vy = (target_y - bomb_y) / FALL_TIME;
            phase = PHASE_FALLING;
            phase_time = 0.0f;
        }
    } else if (phase == PHASE_FALLING) {
        if (phase_time >= FALL_TIME) {
            last_blocked = (holding == correct_method());
            resolve_cx = player.x + PLAYER_W * 0.5f;
            if (last_blocked) {
                resolve_cy = player.y - (float)BLOCK_H * 0.5f - 4.0f;
            } else {
                player_damage();
                resolve_cy = player.y + PLAYER_H * 0.5f;
            }
            phase = PHASE_RESOLVE;
            phase_time = 0.0f;
        }
    } else if (phase == PHASE_RESOLVE) {
        if (phase_time >= RESOLVE_TIME) {
            round_idx++;
            phase = (round_idx >= METHOD_COUNT) ? PHASE_DONE : PHASE_INTRO;
            phase_time = 0.0f;
        }
    }
}

bool webserver_finished(void) {
    return phase == PHASE_DONE;
}

static void fill_circle(SDL_Renderer *r, int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)sqrtf((float)(radius * radius - dy * dy));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

// 텍스트를 (cx, cy) 중심으로 렌더 — datacomm/ui_settings 와 동일 패턴
static void draw_text_centered(SDL_Renderer *r, TTF_Font *font,
                               const char *text, int cx, int cy,
                               SDL_Color color) {
    if (!font || !text) return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst = { cx - surf->w / 2, cy - surf->h / 2, surf->w, surf->h };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

static void draw_block(SDL_Renderer *r, int idx, int x, int y) {
    SDL_SetRenderDrawColor(r, 0, 60, 70, 255);
    SDL_Rect rect = { x, y, BLOCK_W, BLOCK_H };
    SDL_RenderFillRect(r, &rect);
    SDL_SetRenderDrawColor(r, 200, 240, 245, 255);
    SDL_RenderDrawRect(r, &rect);

    SDL_Color white = { 255, 255, 255, 255 };
    draw_text_centered(r, token_font, METHOD_DATA[idx].token,
                       x + BLOCK_W / 2, y + BLOCK_H / 2, white);
}

static void draw_bomb(SDL_Renderer *r) {
    int bx = (int)(player.x + PLAYER_W * 0.5f);
    int by = (int)bomb_y;
    SDL_SetRenderDrawColor(r, 30, 30, 30, 255);
    fill_circle(r, bx, by, BOMB_R);
    // 도화선
    SDL_SetRenderDrawColor(r, 255, 130, 30, 255);
    SDL_Rect fuse = { bx - 5, by - BOMB_R - 16, 10, 16 };
    SDL_RenderFillRect(r, &fuse);
    // 도화선 끝 불꽃
    SDL_SetRenderDrawColor(r, 255, 230, 80, 255);
    fill_circle(r, bx, by - BOMB_R - 19, 7);
}

void webserver_draw(SDL_Renderer *r) {
    // 상단 의미 패널 + 한글 설명
    if (phase == PHASE_INTRO || phase == PHASE_FALLING) {
        int idx = correct_method();
        SDL_SetRenderDrawColor(r, 0, 60, 70, 255);
        SDL_Rect bar = {
            (WINDOW_W - MEANING_BAR_W) / 2,
            MEANING_BAR_Y,
            MEANING_BAR_W,
            MEANING_BAR_H
        };
        SDL_RenderFillRect(r, &bar);
        SDL_SetRenderDrawColor(r, 200, 240, 245, 255);
        SDL_RenderDrawRect(r, &bar);

        SDL_Color white = { 255, 255, 255, 255 };
        draw_text_centered(r, meaning_font, METHOD_DATA[idx].meaning,
                           WINDOW_W / 2, MEANING_BAR_Y + MEANING_BAR_H / 2, white);
    }

    // 바닥 블록 4개 (들고 있는 것은 따로 그림)
    for (int i = 0; i < METHOD_COUNT; i++) {
        if (i == holding) continue;
        draw_block(r, i, (int)METHOD_DATA[i].x, BLOCK_TOP_Y);
    }

    // 들고 있는 블록 — 플레이어 머리 위
    if (holding >= 0) {
        int bx = (int)player.x + ((int)PLAYER_W - BLOCK_W) / 2;
        int by = (int)player.y - BLOCK_H - 6;
        draw_block(r, holding, bx, by);
    }

    if (phase == PHASE_FALLING) {
        draw_bomb(r);
    }

    // 결과 잔상 — 성공은 초록, 실패는 주황 (커지면서 페이드아웃)
    if (phase == PHASE_RESOLVE) {
        float t = phase_time / RESOLVE_TIME;
        if (t > 1.0f) t = 1.0f;
        int radius = (int)(90.0f + 130.0f * t);
        Uint8 alpha = (Uint8)(255.0f * (1.0f - t));
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        if (last_blocked) {
            SDL_SetRenderDrawColor(r, 110, 230, 110, alpha);
        } else {
            SDL_SetRenderDrawColor(r, 255, 150, 60, alpha);
        }
        fill_circle(r, (int)resolve_cx, (int)resolve_cy, radius);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    }
}

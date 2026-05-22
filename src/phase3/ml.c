// 머신러닝 패턴 (김인겸): 로봇 얼굴 + '허거덩'이 타자기 효과로 생성(입 동기)된 뒤 모양 유지한 채
// 낙하, 바닥에 닿으면 자모 7조각으로 깨져 흩어진다. 2초 후 재낙하, 총 3회 반복. 닿으면 데미지.

#include "ml.h"
#include "../player.h"
#include <SDL_ttf.h>

#define W               1280
#define FLOOR_Y         600
#define PLAYER_W        50
#define PLAYER_H        50
#define WORD_LEN        3          // '허','거','덩'
#define REPEAT_COUNT    3          // 낙하 반복 횟수
#define REVEAL_INTERVAL 0.35f      // 글자 생성 간격 (s)
#define FALL_DURATION   1.0f       // 낙하 소요 시간 (s)
#define REST_DURATION   2.0f       // 바닥 도달 후 머무는 시간 (s)
#define TYPING_Y        14.0f      // 글자 생성 중 단어 상단 y
#define PIECE_COUNT     7          // '허거덩' 분해: ㅎ ㅓ ㄱ ㅓ ㄷ ㅓ ㅇ
#define PIECE_GAP       8          // 조각 배치 간격 (px)
#define PIECE_SPREAD_VX 340.0f     // 깨질 때 좌우로 튀는 속도 폭 (px/s)
#define SHATTER_GRAVITY 600.0f     // 깨진 조각 낙하 가속도 (px/s^2)
#define HEAD_X          55         // 로봇 얼굴 (몸통 없이 머리만)
#define HEAD_Y          180
#define HEAD_W          240
#define HEAD_H          260

static const char *glyphs[WORD_LEN]         = { "허", "거", "덩" };
static const char *piece_glyph[PIECE_COUNT] = { "ㅎ","ㅓ","ㄱ","ㅓ","ㄷ","ㅓ","ㅇ" };
// 깨질 때 조각별 초기 상승 속도 (px/s) — 고정값이라 실행마다 동일하게 흩어짐.
static const float piece_vy0[PIECE_COUNT]   =
    { -170.0f, -240.0f, -150.0f, -210.0f, -200.0f, -250.0f, -160.0f };

typedef enum { ML_TYPING, ML_FALLING, ML_RESTING, ML_DONE } MLPhase;
typedef struct { float x, y, vx, vy; bool landed; } Piece;   // 흩어지는 자모 조각

static TTF_Font *font, *jamo_font;
static int   glyph_w[WORD_LEN], glyph_off[WORD_LEN];
static int   word_width, word_height;
static int   piece_w[PIECE_COUNT], piece_h[PIECE_COUNT];
static SDL_Texture *glyph_tex[WORD_LEN], *piece_tex[PIECE_COUNT];
static bool  tex_ready;
static MLPhase phase;
static int   repeat_index, revealed;
static float word_x, word_y, fall_speed;
static float reveal_timer, rest_timer, mouth_open;
static Piece pieces[PIECE_COUNT];

// 단어 너비가 화면 가로 1/3이 되도록 폰트 크기를 산출하고 글자·조각 치수를 측정.
static void load_font(void) {
    if (font) return;
    const char *path = "C:/Windows/Fonts/malgun.ttf";
    int size = 100;
    TTF_Font *probe = TTF_OpenFont(path, 100);
    if (probe) {
        TTF_SetFontStyle(probe, TTF_STYLE_BOLD);
        int pw = 0;
        TTF_SizeUTF8(probe, "허거덩", &pw, NULL);
        TTF_CloseFont(probe);
        if (pw > 0) size = 100 * (W / 3) / pw;
        if (size < 20) size = 20;
    }
    font = TTF_OpenFont(path, size);
    if (!font) return;
    TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    word_height = TTF_FontHeight(font);
    int cursor = 0;
    for (int i = 0; i < WORD_LEN; i++) {
        glyph_off[i] = cursor;
        TTF_SizeUTF8(font, glyphs[i], &glyph_w[i], NULL);
        cursor += glyph_w[i];
    }
    word_width = cursor;
    int jamo_size = (int)(size * 0.55f);   // 조각은 단어보다 작게
    if (jamo_size < 16) jamo_size = 16;
    jamo_font = TTF_OpenFont(path, jamo_size);
    if (jamo_font) {
        TTF_SetFontStyle(jamo_font, TTF_STYLE_BOLD);
        for (int i = 0; i < PIECE_COUNT; i++)
            TTF_SizeUTF8(jamo_font, piece_glyph[i], &piece_w[i], &piece_h[i]);
    }
}

// repeat_index번째 '허거덩'을 로봇을 피한 가로 범위 안 다른 위치에서 생성 시작.
static void spawn_word(void) {
    static const float t[REPEAT_COUNT] = { 0.0f, 1.0f, 0.5f };
    float lo = 320.0f, hi = (float)W - (float)word_width;
    if (hi < lo) hi = lo;
    word_x = lo + (hi - lo) * t[repeat_index];
    word_y = TYPING_Y;
    reveal_timer = rest_timer = mouth_open = 0.0f;
    revealed = 1;                  // 첫 글자 '허' 즉시, 나머지는 0.35초 간격
    phase = ML_TYPING;
}

void ml_start(void) {
    load_font();
    repeat_index = 0;
    if (!font) { phase = ML_DONE; return; }   // 폰트 없으면 패턴 건너뜀
    spawn_word();
}

// 바닥에 닿은 '허거덩'을 7개 자모 조각으로 깨뜨림 — 충격점에서 위·옆으로 튐.
static void shatter_word(void) {
    int total_w = PIECE_GAP * (PIECE_COUNT - 1);
    for (int i = 0; i < PIECE_COUNT; i++) total_w += piece_w[i];
    float row_cy = word_y + word_height * 0.5f;
    float cur    = word_x + word_width * 0.5f - total_w * 0.5f;
    float half   = (PIECE_COUNT - 1) * 0.5f;
    for (int i = 0; i < PIECE_COUNT; i++) {
        Piece *p = &pieces[i];
        p->x = cur;
        p->y = row_cy - piece_h[i] * 0.5f;
        cur += piece_w[i] + PIECE_GAP;
        p->vx = ((i - half) / half) * PIECE_SPREAD_VX;
        p->vy = piece_vy0[i];
        p->landed = false;
    }
}

// 사각형(x,y,w,h)이 플레이어와 겹치는가 (AABB).
static bool hits_player(float x, float y, float w, float h) {
    return player.x < x + w && player.x + PLAYER_W > x &&
           player.y < y + h && player.y + PLAYER_H > y;
}

// 떨어지는 단어 / 깨진 조각이 플레이어와 겹치면 데미지 (무적은 player_damage 내부 처리).
static void check_collision(void) {
    if (phase == ML_FALLING) {
        for (int i = 0; i < WORD_LEN; i++)
            if (hits_player(word_x + glyph_off[i], word_y, glyph_w[i], word_height)) {
                player_damage();
                return;
            }
    } else if (phase == ML_RESTING) {
        for (int i = 0; i < PIECE_COUNT; i++)
            if (hits_player(pieces[i].x, pieces[i].y, piece_w[i], piece_h[i])) {
                player_damage();
                return;
            }
    }
}

void ml_update(float dt) {
    if (!font || phase == ML_DONE) return;
    if (phase == ML_TYPING) {
        reveal_timer += dt;
        while (reveal_timer >= REVEAL_INTERVAL && revealed < WORD_LEN) {
            reveal_timer -= REVEAL_INTERVAL;
            revealed++;
        }
        // 입 애니메이션 — 글자 생성 주기에 맞춰 삼각파로 열고 닫음
        float f = reveal_timer / REVEAL_INTERVAL;
        mouth_open = (f < 0.5f) ? f * 2.0f : 2.0f - f * 2.0f;
        if (revealed >= WORD_LEN) {
            phase = ML_FALLING;
            mouth_open = 0.0f;
            fall_speed = (FLOOR_Y - word_height - word_y) / FALL_DURATION;
        }
    } else if (phase == ML_FALLING) {
        word_y += fall_speed * dt;
        if (word_y + word_height >= FLOOR_Y) {
            word_y = FLOOR_Y - word_height;
            shatter_word();
            phase = ML_RESTING;
            rest_timer = 0.0f;
        }
    } else if (phase == ML_RESTING) {
        for (int i = 0; i < PIECE_COUNT; i++) {
            Piece *p = &pieces[i];
            if (p->landed) continue;
            p->vy += SHATTER_GRAVITY * dt;
            p->x  += p->vx * dt;
            p->y  += p->vy * dt;
            if (p->x < 0)                   { p->x = 0;              p->vx = 0; }
            else if (p->x > W - piece_w[i]) { p->x = W - piece_w[i]; p->vx = 0; }
            if (p->y + piece_h[i] >= FLOOR_Y) {
                p->y = FLOOR_Y - piece_h[i];
                p->vx = p->vy = 0;
                p->landed = true;
            }
        }
        rest_timer += dt;
        if (rest_timer >= REST_DURATION) {
            if (++repeat_index >= REPEAT_COUNT) { phase = ML_DONE; return; }
            spawn_word();
            return;
        }
    }
    check_collision();
}

bool ml_finished(void) { return phase == ML_DONE; }

// 색을 지정해 사각형을 채움.
static void fillc(SDL_Renderer *r, int cr, int cg, int cb, int x, int y, int w, int h) {
    SDL_SetRenderDrawColor(r, cr, cg, cb, 255);
    SDL_Rect rc = { x, y, w, h };
    SDL_RenderFillRect(r, &rc);
}

static void blit(SDL_Renderer *r, SDL_Texture *t, int x, int y, int w, int h) {
    if (!t) return;
    SDL_Rect dst = { x, y, w, h };
    SDL_RenderCopy(r, t, NULL, &dst);
}

// 머신러닝 보스 — 로봇 얼굴만. mouth(0~1)에 따라 입 높이가 변해 말하는 효과.
static void draw_robot(SDL_Renderer *r, float mouth) {
    int cx = HEAD_X + HEAD_W / 2;
    fillc(r,  75, 100, 115, cx - 6,  HEAD_Y - 44, 12, 48);   // 안테나 막대
    fillc(r,   0, 235, 235, cx - 18, HEAD_Y - 74, 36, 32);   // 안테나 끝
    fillc(r, 115, 140, 155, HEAD_X, HEAD_Y, HEAD_W, HEAD_H); // 머리
    fillc(r,  60,  78,  92, HEAD_X + 22, HEAD_Y + 22, HEAD_W - 44, HEAD_H - 44);  // 얼굴 패널
    int ew = 58, eh = 64, ey = HEAD_Y + 64;
    int elx = HEAD_X + 36, erx = HEAD_X + HEAD_W - 36 - ew;
    fillc(r,   0, 235, 235, elx, ey, ew, eh);                // 눈 (좌)
    fillc(r,   0, 235, 235, erx, ey, ew, eh);                // 눈 (우)
    fillc(r, 200, 255, 255, elx + 14, ey + 16, 20, 22);      // 눈 강조 (좌)
    fillc(r, 200, 255, 255, erx + 14, ey + 16, 20, 22);      // 눈 강조 (우)
    int mw = 150, mh = 12 + (int)(mouth * 58.0f);
    int mx = cx - mw / 2, my = HEAD_Y + HEAD_H - 64 - mh / 2;
    fillc(r, 25, 35, 42, mx, my, mw, mh);                    // 입 안쪽
    for (int gx = mx + 16; gx < mx + mw - 8; gx += 22)       // 입 그릴 (시안 세로선)
        fillc(r, 0, 200, 200, gx, my + 3, 5, mh - 6);
}

// 문자열 배열을 텍스처로 변환 (최초 1회).
static void make_textures(SDL_Renderer *r, TTF_Font *f, const char **strs,
                          int n, SDL_Texture **out, SDL_Color c) {
    if (!f) return;
    for (int i = 0; i < n; i++) {
        SDL_Surface *s = TTF_RenderUTF8_Blended(f, strs[i], c);
        if (!s) continue;
        out[i] = SDL_CreateTextureFromSurface(r, s);
        SDL_FreeSurface(s);
        if (out[i]) SDL_SetTextureBlendMode(out[i], SDL_BLENDMODE_BLEND);
    }
}

static void ensure_textures(SDL_Renderer *r) {
    if (tex_ready || !font) return;
    SDL_Color word_c = { 215, 255, 255, 255 }, piece_c = { 190, 255, 255, 255 };
    make_textures(r, font,      glyphs,      WORD_LEN,    glyph_tex, word_c);
    make_textures(r, jamo_font, piece_glyph, PIECE_COUNT, piece_tex, piece_c);
    tex_ready = true;
}

void ml_draw(SDL_Renderer *r) {
    draw_robot(r, mouth_open);
    if (!font || phase == ML_DONE) return;
    ensure_textures(r);
    if (phase == ML_RESTING) {     // 깨져 흩어진 자모 조각
        for (int i = 0; i < PIECE_COUNT; i++)
            blit(r, piece_tex[i], (int)pieces[i].x, (int)pieces[i].y, piece_w[i], piece_h[i]);
    } else {                       // 생성·낙하 중인 '허거덩' (모양 유지)
        for (int i = 0; i < revealed; i++)
            blit(r, glyph_tex[i], (int)word_x + glyph_off[i], (int)word_y, glyph_w[i], word_height);
    }
}

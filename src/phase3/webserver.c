#include "webserver.h"
#include "../player.h"
#include <SDL_ttf.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define LOGO_CX (WINDOW_W / 2)
#define LOGO_CY 85
#define LOGO_H 140                                // 로고(육각형) 전체 높이
#define DROP_Y 200.0f   // 로고 하단 + 여백 — 모든 링크 시작 y
#define FALL_SPEED 300.0f
#define HOMING_SPEED 420.0f
#define MAX_FALLERS 32
#define CELL_W 56           // 글자 하나가 차지하는 가로 폭 — 늘려 화면을 채움

// 낙하 중인 글자 묶음 하나 (링크 전체 / 절반 / 등분 조각 / 글자 한 개)
typedef struct {
    SDL_Texture *tex;
    int   w, h;
    float x, y, vx, vy;
    float delay;       // 턴 시작 후 떨어지기 시작하는 시각
    bool  launched;
    bool  gone;
} Faller;

static const char *URLS[4] = {
    "https://www.naver.com",
    "https://www.youtube.com",
    "https://chatgpt.com",
    "https://www.sungkyul.ac.kr",
};

static TTF_Font *font;
static Faller fallers[MAX_FALLERS];
static int faller_count;
static int turn;
static float timer;
static bool built;
static bool done;

void webserver_start(void) {
    if (!font) font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 40);
    turn  = 0;
    timer = -2.0f;   // 첫 링크도 로고 등장 후 2초 대기
    built = false;
    done  = false;
}

// URL의 [start, start+len) 글자를 텍스처로 구워 cx 중심에 놓는 Faller 추가
static void add_faller(SDL_Renderer *r, const char *s, int start, int len,
                       float cx, float delay) {
    char buf[64];
    memcpy(buf, s + start, len);
    buf[len] = '\0';

    SDL_Color col = { 150, 230, 245, 255 };
    SDL_Surface *sf = TTF_RenderUTF8_Blended(font, buf, col);

    Faller *f = &fallers[faller_count++];
    f->tex = SDL_CreateTextureFromSurface(r, sf);
    f->w = len * CELL_W;                        // 글자 수 × 폭 — 가로로 늘임
    f->h = (int)((float)sf->h * f->w / sf->w);  // 가로 확대율만큼 세로도 — 비율 유지
    SDL_FreeSurface(sf);
    f->x = cx - f->w / 2.0f;
    f->y = DROP_Y;
    f->vx = 0.0f;
    f->vy = FALL_SPEED;
    f->delay = delay;
    f->launched = false;
    f->gone = false;
}

// 현재 턴의 Faller 구성 — 텍스처 생성에 렌더러가 필요해 draw에서 호출
static void build_turn(SDL_Renderer *r) {
    for (int i = 0; i < faller_count; i++) SDL_DestroyTexture(fallers[i].tex);
    faller_count = 0;

    const char *url = URLS[turn];
    int n = (int)strlen(url);

    if (turn == 0) {
        add_faller(r, url, 0, n, WINDOW_W / 2, 0.0f);
    }
    else if (turn == 1) {
        int half = (n + 1) / 2;  // 앞쪽이 한 글자 더 — "youtube.com" 이 뒤 조각
        add_faller(r, url, 0, half, half * CELL_W / 2.0f, 0.0f); // 왼쪽 끝에 닿게
        add_faller(r, url, half, n - half,
                   WINDOW_W - (n - half) * CELL_W / 2.0f, 2.2f); // 오른쪽 끝에 닿게
    }
    else if (turn == 2) {
        int lens[3] = { 8, 7, n - 15 };   // "https://" / "chatgpt" / ".com"
        int start = 0;
        for (int i = 0; i < 3; i++) {
            add_faller(r, url, start, lens[i], WINDOW_W / 2, i * 2.0f);  // 조각마다 2초 간격 발사
            start += lens[i];
        }
    }
    else {
        for (int i = 0; i < n; i++) {
            float cx = (float)(60 + rand() % (WINDOW_W - 120));
            add_faller(r, url, i, 1, cx, i * 0.20f);
        }
    }
    built = true;
}

void webserver_update(float dt) {
    if (done || !built) return;
    timer += dt;

    int alive = 0;
    for (int i = 0; i < faller_count; i++) {
        Faller *f = &fallers[i];
        if (f->gone) continue;
        alive++;

        if (!f->launched) {
            if (timer < f->delay) continue;
            f->launched = true;
            if (turn == 2) {   // 발사 순간 플레이어 위치를 한 번만 조준
                float dx = player.x + PLAYER_W / 2.0f - (f->x + f->w / 2.0f);
                float dy = player.y + PLAYER_H / 2.0f - (f->y + f->h / 2.0f);
                float d = sqrtf(dx * dx + dy * dy);
                f->vx = dx / d * HOMING_SPEED;
                f->vy = dy / d * HOMING_SPEED;
            }
        }

        f->x += f->vx * dt;
        f->y += f->vy * dt;

        // 글자 묶음(사각) vs 플레이어 AABB 충돌 (무적 처리는 player_damage 내부)
        if (!(f->x + f->w < player.x || f->x > player.x + PLAYER_W ||
              f->y + f->h < player.y || f->y > player.y + PLAYER_H)) {
            player_damage();
        }

        if (f->y > WINDOW_H + 40.0f || f->x + f->w < -40.0f || f->x > WINDOW_W + 40.0f) {
            f->gone = true;
        }
    }

    if (alive == 0) {
        turn++;
        timer = -2.0f;   // 다음 링크까지 2초 대기 (timer 음수 동안 delay 게이트가 막음)
        built = false;
        if (turn >= 4) done = true;
    }
}

bool webserver_finished(void) {
    return done;
}

// 위·아래 꼭짓점 육각형 채우기 — 가운데 띠 최대폭, 위아래로 좁아짐
static void fill_hexagon(SDL_Renderer *r, int cx, int cy, int w, int h) {
    int hw = w / 2, hh = h / 2, flat = h / 4;
    for (int dy = -hh; dy <= hh; dy++) {
        int ady = dy < 0 ? -dy : dy;
        int span = ady <= flat ? hw
            : (int)(hw * (1.0f - (float)(ady - flat) / (hh - flat)));
        SDL_RenderDrawLine(r, cx - span, cy + dy, cx + span, cy + dy);
    }
}

static void draw_logo(SDL_Renderer *r) {
    SDL_SetRenderDrawColor(r, 60, 100, 30, 255);
    fill_hexagon(r, LOGO_CX, LOGO_CY, 116, 140);
    SDL_SetRenderDrawColor(r, 131, 205, 41, 255);
    fill_hexagon(r, LOGO_CX, LOGO_CY, 96, 116);

    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Surface *sf = TTF_RenderUTF8_Blended(font, "JS", white);
    int jw = sf->w, jh = sf->h;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, sf);
    SDL_FreeSurface(sf);
    SDL_Rect d = { LOGO_CX - jw * 3 / 4, LOGO_CY - jh * 3 / 4,
                   jw * 3 / 2, jh * 3 / 2 };
    SDL_RenderCopy(r, t, NULL, &d);
    SDL_DestroyTexture(t);
}

void webserver_draw(SDL_Renderer *r) {
    if (!done && !built) build_turn(r);

    if (!done) {
        for (int i = 0; i < faller_count; i++) {
            Faller *f = &fallers[i];
            if (!f->launched || f->gone) continue;
            SDL_Rect d = { (int)f->x, (int)f->y, f->w, f->h };
            SDL_RenderCopy(r, f->tex, NULL, &d);
        }
    }
    draw_logo(r);
}
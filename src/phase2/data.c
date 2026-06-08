// 데이터통신 패턴 (2학년)
// OSI 7계층 블럭이 패턴 시작 시 한 번에 세로로 배치된다.
// 플레이어는 가장 아래 Physical 부터 위 Application 까지 순서대로 밟아야 한다.
// 다음 계층을 건너뛰면 HP 감소. 이미 클리어한 계층으로 되돌아가는 건 페널티 없음.
//
// 기본 바닥(FLOOR_Y)을 그대로 사용한다 — floor 토글을 건드리지 않음.

#include "data.h"
#include "../player.h"
#include <SDL_ttf.h>

#if defined(_WIN32)
#define DATA_FONT_PATH "C:/Windows/Fonts/consola.ttf"
#elif defined(__APPLE__)
#define DATA_FONT_PATH "/Library/Fonts/Arial Unicode.ttf"
#else
#define DATA_FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#endif

#define LAYER_COUNT       7
#define BLOCK_W         320
#define BLOCK_H          40

// 블럭은 창의 왼쪽 끝(x=0)과 오른쪽 끝(x=WINDOW_W - BLOCK_W=960)까지 왕복.
#define BLOCK_X_MIN     0.0f
#define BLOCK_X_MAX     ((float)(WINDOW_W - BLOCK_W))

typedef struct {
    const char *label;
    float x;           // 현재 블럭 좌상단 x
    float vx;          // 좌우 속도 (px/s). 정확히 밟히면 0 으로 멈춤.
    float y;
    bool  cleared;     // 정확한 순서로 한 번이라도 밟았는지
} OsiBlock;

// 아래(Physical) → 위(Application). y 가 클수록 화면 아래.
// 초기 x 와 속도는 블럭별로 달라 동일 위상으로 움직이지 않음.
static const struct {
    const char *label;
    float x;
    float y;
    float vx;
} osi_spec[LAYER_COUNT] = {
    { "Physical",      140.0f, 540.0f,  120.0f },
    { "Data Link",     480.0f, 460.0f, -100.0f },
    { "Network",       820.0f, 380.0f,  140.0f },
    { "Transport",     140.0f, 300.0f, -110.0f },
    { "Session",       480.0f, 220.0f,  130.0f },
    { "Presentation",  820.0f, 140.0f, -120.0f },
    { "Application",   140.0f,  60.0f,  100.0f },
};

static OsiBlock blocks[LAYER_COUNT];
static int  expected_next;       // 다음으로 밟아야 할 인덱스 (0 = Physical)
static int  prev_block_idx;      // 직전 프레임에 밟고 있던 블럭 (-1 = 공중/바닥)
static TTF_Font *layer_font = NULL;

static void load_font(void) {
    if (layer_font) return;
    layer_font = TTF_OpenFont(DATA_FONT_PATH, 22);
    if (!layer_font) {
#if defined(_WIN32)
        layer_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 22);
#elif defined(__APPLE__)
        layer_font = TTF_OpenFont("/Library/Fonts/Arial Unicode.ttf", 22);
#else
        layer_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 22);
#endif
    }
}

void data_start(void) {
    for (int i = 0; i < LAYER_COUNT; i++) {
        blocks[i].label   = osi_spec[i].label;
        blocks[i].x       = osi_spec[i].x;
        blocks[i].vx      = osi_spec[i].vx;
        blocks[i].y       = osi_spec[i].y;
        blocks[i].cleared = false;
    }
    expected_next  = 0;
    prev_block_idx = -1;
    load_font();
}

static bool player_over_block(const OsiBlock *b) {
    float px_center = player.x + (float)(PLAYER_W / 2);
    if (px_center <  b->x)             return false;
    if (px_center >= b->x + BLOCK_W)   return false;
    return true;
}

void data_update(float dt) {
    // 직전 프레임에 밟고 있던 블럭이 있으면 그 속도만큼 플레이어를 같이 이동
    // (움직이는 발판 위 자연스럽게 운반)
    if (prev_block_idx != -1) {
        player.x += blocks[prev_block_idx].vx * dt;
    }

    // 블럭 좌우 진동 — 창 양 끝에 닿으면 방향 반전. 클리어된 블럭은 vx=0 이라 정지.
    for (int i = 0; i < LAYER_COUNT; i++) {
        blocks[i].x += blocks[i].vx * dt;
        if (blocks[i].x < BLOCK_X_MIN) { blocks[i].x = BLOCK_X_MIN; blocks[i].vx = -blocks[i].vx; }
        if (blocks[i].x > BLOCK_X_MAX) { blocks[i].x = BLOCK_X_MAX; blocks[i].vx = -blocks[i].vx; }
    }

    // 1방향 플랫폼 충돌 — 떨어지는 중일 때만 발 아래 블럭에 스냅
    int landed_idx = -1;
    if (player.vy >= 0.0f) {
        float feet = player.y + (float)PLAYER_H;
        float best_top = 0.0f;
        for (int i = 0; i < LAYER_COUNT; i++) {
            if (!player_over_block(&blocks[i])) continue;
            float top = blocks[i].y;
            float bot = top + (float)BLOCK_H;
            if (feet < top || feet > bot) continue;
            if (landed_idx == -1 || top > best_top) {
                best_top = top;
                landed_idx = i;
            }
        }
        if (landed_idx != -1) {
            player.y = best_top - (float)PLAYER_H;
            player.vy = 0.0f;
            player.on_ground = true;
            player.jump_count = 0;
        }
    }

    // 새로 다른 블럭을 밟은 시점에만 순서 판정
    if (landed_idx != -1 && landed_idx != prev_block_idx) {
        if (landed_idx == expected_next) {
            blocks[landed_idx].cleared = true;
            blocks[landed_idx].vx = 0.0f;   // 올바르게 밟힌 블럭은 정지
            // 위쪽(아직 안 밟은) 발판들은 속도 1.35배로 가속 — 난이도 점증
            for (int j = landed_idx + 1; j < LAYER_COUNT; j++) {
                blocks[j].vx *= 1.35f;
            }
            expected_next++;
        } else if (landed_idx > expected_next) {
            // 다음 계층을 건너뛰었음 → HP 감소
            player_damage();
        }
        // landed_idx < expected_next : 이미 클리어한 계층 — 페널티 없음
    }
    prev_block_idx = landed_idx;
}

bool data_finished(void) {
    return expected_next >= LAYER_COUNT;
}

void data_draw(SDL_Renderer *r) {
    for (int i = 0; i < LAYER_COUNT; i++) {
        SDL_Rect rect = { (int)blocks[i].x, (int)blocks[i].y, BLOCK_W, BLOCK_H };

        if (blocks[i].cleared) {
            SDL_SetRenderDrawColor(r,   0,  80, 100, 255);   // 클리어 — 어두운 시안
        } else {
            SDL_SetRenderDrawColor(r,   0, 140, 170, 255);   // 미클리어 — 밝은 시안
        }
        SDL_RenderFillRect(r, &rect);
        SDL_SetRenderDrawColor(r, 0, 220, 240, 255);
        SDL_RenderDrawRect(r, &rect);

        if (layer_font && blocks[i].label) {
            SDL_Color white = { 255, 255, 255, 255 };
            SDL_Surface *surf = TTF_RenderUTF8_Blended(layer_font, blocks[i].label, white);
            if (surf) {
                SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
                SDL_Rect dst = {
                    (int)blocks[i].x + (BLOCK_W - surf->w) / 2,
                    (int)blocks[i].y + (BLOCK_H - surf->h) / 2,
                    surf->w, surf->h
                };
                SDL_RenderCopy(r, tex, NULL, &dst);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(tex);
            }
        }
    }
}

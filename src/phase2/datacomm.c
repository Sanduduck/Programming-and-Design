#include "datacomm.h"
#include "../player.h"
#include <SDL_ttf.h>

#define WINDOW_W 1280

#define LAYER_COUNT 7 // OSI 7계층
#define BLOCK_W 320
#define BLOCK_H 40

// 발판 x 이동 범위 — 왼쪽 끝 0, 오른쪽 끝은 (창 폭 - 블럭 폭) 까지 (블럭 우측이 창에 닿는 지점)
#define BLOCK_X_MIN 0.0f
#define BLOCK_X_MAX (float)(WINDOW_W - BLOCK_W)

// 발판 한 칸의 런타임 상태
typedef struct {
    const char *label;
    float x;
    float vx; // 좌우 속도 (px/s). 정확히 밟히면 0 으로 멈춤.
    float y;
    bool cleared; // 정확한 순서로 한 번이라도 밟았는지
} OsiBlock;

// 발판별 초기 스펙 — 인덱스 0=Physical(맨 아래), 6=Application(맨 위)
static const struct {
    const char *label;
    float x;
    float y;
    float vx;
} osi_spec[LAYER_COUNT] = {
    { "Physical", 140.0f, 540.0f, 120.0f },   // L1 — 시작 발판, 오른쪽으로 이동
    { "Data Link", 480.0f, 460.0f, -100.0f },   // L2
    { "Network", 820.0f, 380.0f, 140.0f },   // L3
    { "Transport", 140.0f, 300.0f, -110.0f },   // L4
    { "Session", 480.0f, 220.0f, 130.0f },   // L5
    { "Presentation", 820.0f, 40.0f, -120.0f },   // L6
    { "Application", 140.0f, 60.0f,  100.0f },   // L7 — 최상단, 도달 시 패턴 종료
};

// 런타임 상태 — datacomm_start 에서 매번 리셋
static OsiBlock blocks[LAYER_COUNT];
static int expected_next;            // 다음으로 밟아야 할 발판 인덱스 (0 → 6 순서)
static int prev_block_idx;           // 직전 프레임에 밟고 있던 발판 (-1 = 공중)
static TTF_Font *layer_font = NULL;

// 폰트는 최초 한 번만 로드 — 이미 로드돼 있으면 재호출해도 무비용
static void load_font(void) {
    if (layer_font) return;
    layer_font = TTF_OpenFont("C:/Windows/Fonts/consola.ttf", 22);
    }

// 패턴 진입 시 호출 — 발판/진행 상태 전체 초기화
void datacomm_start(void) {
    // 발판을 설정값대로 리셋, cleared 플래그도 모두 false
    for (int i = 0; i < LAYER_COUNT; i++) {
        blocks[i].label = osi_spec[i].label;
        blocks[i].x = osi_spec[i].x;
        blocks[i].vx = osi_spec[i].vx;
        blocks[i].y = osi_spec[i].y;
        blocks[i].cleared = false;
    }
    expected_next = 0;       // Physical부터 시작
    prev_block_idx = -1;      // 시작 시점에는 공중 취급
    load_font();
}

// 플레이어 중심 x 가 발판의 x 범위 안에 들어와 있는지 (좌우 겹침만 판정)
static bool player_over_block(const OsiBlock *b) {
    float px_center = player.x + (float)(PLAYER_W / 2);
    if (px_center < b->x) return false;   // 발판 왼쪽 밖
    if (px_center >= b->x+ BLOCK_W) return false;   // 발판 오른쪽 밖
    return true;
}

// 매 프레임 호출 — 발판 이동, 착지 판정, 순서 판정
void datacomm_update(float dt) {
    // 직전에 밟고 있던 발판이 있으면 그 속도만큼 플레이어를 같이 이동
    if (prev_block_idx != -1) {
        player.x += blocks[prev_block_idx].vx * dt;
    }

    // 모든 발판의 좌우 진동 — 창 양 끝에 닿으면 vx 부호 반전
    // 클리어된 발판은 vx=0 으로 고정돼 있어 사실상 정지
    for (int i = 0; i < LAYER_COUNT; i++) {
        blocks[i].x += blocks[i].vx * dt;
        if (blocks[i].x < BLOCK_X_MIN) { blocks[i].x = BLOCK_X_MIN; blocks[i].vx = -blocks[i].vx; }
        if (blocks[i].x > BLOCK_X_MAX) { blocks[i].x = BLOCK_X_MAX; blocks[i].vx = -blocks[i].vx; }
    }

    // 1방향 플랫폼 충돌 — 떨어지는 중(vy >= 0)일 때만 발판 위에 스냅
    // 올라가는 도중(vy < 0)에는 발판을 그대로 통과 (점프로 위층 진입 가능)
    int landed_idx = -1;
    if (player.vy >= 0.0f) {
        float feet = player.y + (float)PLAYER_H;
        float best_top = 0.0f;

        // 발끝이 발판 두께 사이를 관통하는 발판들 중 가장 위(=화면 아래)의 것 선택
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

        // 착지 — 발판 상단에 스냅하고 점프 카운트 리셋
        if (landed_idx != -1) {
            player.y = best_top - (float)PLAYER_H;
            player.vy = 0.0f;
            player.on_ground = true;
            player.jump_count = 0;
        }
    }

    // 새로 다른 발판을 밟은 프레임에만 순서 판정 트리거
    if (landed_idx != -1 && landed_idx != prev_block_idx) {
        if (landed_idx == expected_next) {
            // 정확한 순서 — 클리어 처리 + 발판 정지 (이후 안정 발판 역할)
            blocks[landed_idx].cleared = true;
            blocks[landed_idx].vx = 0.0f;
            expected_next++;
        } else if (landed_idx > expected_next) {
            // 계층을 건너뛰었음 → HP 감소 (무적 처리는 player_damage 내부에서)
            player_damage();
        }
        // landed_idx < expected_next : 이미 클리어한 계층으로 되돌아옴 — 페널티 없음
    }

    // 다음 프레임 비교용으로 현재 착지 인덱스 저장
    prev_block_idx = landed_idx;
}

// 패턴 종료 조건 — Application 까지 모두 클리어
bool datacomm_finished(void) {
    return expected_next >= LAYER_COUNT;
}

// 매 프레임 호출 — 모든 발판 + 계층명 텍스트 렌더링
void datacomm_draw(SDL_Renderer *r) {
    for (int i = 0; i < LAYER_COUNT; i++) {
        SDL_Rect rect = { (int)blocks[i].x, (int)blocks[i].y, BLOCK_W, BLOCK_H };

        // 클리어 여부에 따라 채움색 분기 — 진행 상황을 색으로 표시
        if (blocks[i].cleared) {
            SDL_SetRenderDrawColor(r, 0, 80, 100, 255);   // 클리어 — 어두운 색
        } else {
            SDL_SetRenderDrawColor(r, 0, 140, 170, 255);   // 미클리어 — 밝은 색
        }
        SDL_RenderFillRect(r, &rect);

        // 발판 외곽선 (밝은 시안) — 경계 명확화
        SDL_SetRenderDrawColor(r, 0, 220, 240, 255);
        SDL_RenderDrawRect(r, &rect);

        // 발판 중앙에 계층 이름 출력
        if (layer_font && blocks[i].label) {
            SDL_Color white = { 255, 255, 255, 255 };
            SDL_Surface *surf = TTF_RenderUTF8_Blended(layer_font, blocks[i].label, white);
            if (surf) {
                SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
                // 발판 크기와 텍스트 크기의 차이를 절반씩 양쪽에 분배 → 가운데 정렬
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

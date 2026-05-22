// 패턴 디자인 회피 패턴: 화면 상단 명령어에 따라 칸으로 이동하는 게임
// random() 혹은 time() 지시어가 뜨면 지정된 칸으로 2초 안에 이동해야 합니다.

#include "pdesign.h"
#include "../player.h"
#include <SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#if defined(_WIN32)
#define PD_FONT_PATH "C:/Windows/Fonts/consola.ttf"
#elif defined(__APPLE__)
#define PD_FONT_PATH "/Library/Fonts/Arial Unicode.ttf"
#else
#define PD_FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#endif

#define WINDOW_W         1280  // 전체 윈도우 너비
#define WINDOW_H          720  // 전체 윈도우 높이
#define FLOOR_Y           600  // 바닥 위치 (향후 확장용)
#define PD_PLAYER_W       50   // 플레이어 너비
#define PD_PLAYER_H       50   // 플레이어 높이
#define GRID_COLS          5   // 그리드 열 개수
#define GRID_ROWS          1   // 그리드 행 개수
#define CELL_W            200  // 셀 너비
#define CELL_H             80  // 셀 높이
#define CELL_MARGIN        28   // 셀 간 간격
#define GRID_LEFT          90   // 그리드 시작 X 좌표
#define GRID_TOP          460  // 그리드 시작 Y 좌표
#define PROMPT_TIME        2.0f // 명령 선택 후 이동 가능 시간
#define WAIT_TIME          1.0f // 다음 프롬프트 대기 시간
#define MAX_PD_ROUNDS      5   // 최대 반복 라운드 수

typedef enum {
    PD_STATE_WAITING, // 명령 대기 상태
    PD_STATE_ACTIVE,  // 명령 활성화 상태
    PD_STATE_DONE     // 모든 라운드 완료 상태
} PDesignState;

typedef struct {
    int x, y;         // 셀 위치
    int number;       // 셀 번호 표시용
    bool visible;     // 해당 셀을 화면에 그릴지 여부
    bool target;      // 정답 셀인지 여부
} PDesignCell;

static PDesignCell cells[GRID_ROWS * GRID_COLS]; // 그리드 셀 배열
static TTF_Font *pd_font = NULL;                   // 일반 텍스트 폰트
static TTF_Font *pd_prompt_font = NULL;            // 상단 프롬프트 폰트
static float prompt_timer;                         // 대기/타이머 경과 시간
static PDesignState state;                         // 현재 상태
static int target_index;                           // 정답 셀 인덱스
static int round_count;                            // 현재 라운드 카운트
static bool seeded = false;                        // srand() 초기화 여부
static bool grid_active;                           // 그리드 활성화 플래그

// 폰트를 로드하고 초기화한다. 이미 로드되어 있으면 재사용한다.
static void load_font(void) {
    if (pd_font && pd_prompt_font) return;
    if (!pd_font) {
        pd_font = TTF_OpenFont(PD_FONT_PATH, 24);
        if (!pd_font) {
            SDL_Log("Failed to load font %s: %s", PD_FONT_PATH, TTF_GetError());
        }
    }
    if (!pd_prompt_font) {
        pd_prompt_font = TTF_OpenFont(PD_FONT_PATH, 40);
        if (!pd_prompt_font) {
            SDL_Log("Failed to load prompt font %s: %s", PD_FONT_PATH, TTF_GetError());
        }
    }
}

// 플레이어의 중심 X 위치가 해당 셀 영역 안에 들어가는지 확인한다.
static bool player_over_cell(const PDesignCell *cell) {
    float px_center = player.x + (PD_PLAYER_W / 2.0f);
    return (px_center >= cell->x && px_center < cell->x + CELL_W);
}

// 플레이어가 현재 선택된 타겟 셀 위에 위치하는지 검사한다.
static bool player_on_target(void) {
    PDesignCell *cell = &cells[target_index];
    float px = player.x;
    float py = player.y;
    return (px + PD_PLAYER_W > cell->x && px < cell->x + CELL_W &&
            py + PD_PLAYER_H > cell->y && py < cell->y + CELL_H);
}

// 모든 셀을 초기 위치와 기본 상태로 초기화한다.
static void reset_cells(void) {
    int idx = 0;
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            cells[idx].x = GRID_LEFT + col * (CELL_W + CELL_MARGIN);
            cells[idx].y = GRID_TOP + row * (CELL_H + CELL_MARGIN);
            cells[idx].number = idx + 1;
            cells[idx].visible = true;
            cells[idx].target = false;
            idx++;
        }
    }
}

// 다음 라운드를 위한 정답 셀을 무작위로 선택하고 상태를 활성화한다.
static void choose_prompt(void) {
    target_index = rand() % (GRID_ROWS * GRID_COLS);
    for (int i = 0; i < GRID_ROWS * GRID_COLS; i++) {
        cells[i].visible = true;
        cells[i].target = (i == target_index);
    }
    prompt_timer = 0.0f;
    state = PD_STATE_ACTIVE;
    grid_active = true;
}

// 패턴 디자인 게임을 시작하거나 재시작할 때 호출된다.
void pdesign_start(void) {
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = true;
    }
    load_font();
    reset_cells();
    prompt_timer = 0.0f;
    round_count = 0;
    state = PD_STATE_WAITING;
    grid_active = true;
}

void pdesign_update(float dt) {
    if (state == PD_STATE_DONE) return;

    prompt_timer += dt;

    // WAITING 상태에서는 짧은 대기 후 다음 프롬프트를 선택한다.
    if (state == PD_STATE_WAITING) {
        if (prompt_timer >= WAIT_TIME) {
            choose_prompt();
        }
        return;
    }

    // 플레이어가 아래로 떨어질 때 셀 위에 착지하도록 처리.
    if (player.vy >= 0.0f) {
        int landed_idx = -1;
        float feet = player.y + PD_PLAYER_H;
        float best_top = 0.0f;
        for (int i = 0; i < GRID_ROWS * GRID_COLS; i++) {
            if (!cells[i].visible) continue;
            if (!player_over_cell(&cells[i])) continue;
            float top = (float)cells[i].y;
            float bot = top + CELL_H;
            if (feet < top || feet > bot) continue;
            if (landed_idx == -1 || top > best_top) {
                best_top = top;
                landed_idx = i;
            }
        }
        if (landed_idx != -1) {
            player.y = best_top - PD_PLAYER_H;
            player.vy = 0.0f;
            player.on_ground = true;
            player.jump_count = 0;
        }
    }

    if (state == PD_STATE_ACTIVE) {
        // 플레이어가 정답 셀에 도달하면 성공 처리
        if (player_on_target()) {
            round_count++;
            if (round_count >= MAX_PD_ROUNDS) {
                state = PD_STATE_DONE;
            } else {
                reset_cells();
                state = PD_STATE_WAITING;
                prompt_timer = 0.0f;
            }
            return;
        }

        // 제한 시간이 지나면 실패 처리 후 다음 라운드로 전환
        if (prompt_timer >= PROMPT_TIME) {
            player_damage();
            round_count++;
            if (round_count >= MAX_PD_ROUNDS) {
                state = PD_STATE_DONE;
            } else {
                reset_cells();
                state = PD_STATE_WAITING;
                prompt_timer = 0.0f;
            }
            return;
        }
    }
}

bool pdesign_finished(void) {
    return state == PD_STATE_DONE;
}

// 텍스트를 렌더링하여 화면에 출력한다.
static void draw_text(SDL_Renderer *r, TTF_Font *font, const char *text, int x, int y) {
    if (!font) return;
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, white);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    if (!tex) {
        SDL_FreeSurface(surf);
        return;
    }
    SDL_Rect dst = { x, y, surf->w, surf->h };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

// 패턴 디자인 화면을 그린다.
void pdesign_draw(SDL_Renderer *r) {
    SDL_SetRenderDrawColor(r, 10, 10, 30, 255);
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    SDL_RenderFillRect(r, &bg);

    char prompt_text[64];
    if (state == PD_STATE_WAITING) {
        snprintf(prompt_text, sizeof(prompt_text), "Ready for prompt...");
    } else if (state == PD_STATE_ACTIVE) {
        snprintf(prompt_text, sizeof(prompt_text), "rand() -> red cell");
    } else {
        snprintf(prompt_text, sizeof(prompt_text), "Finished");
    }
    draw_text(r, pd_prompt_font, prompt_text, 40, 40);

    char round_text[64];
    if (state == PD_STATE_DONE) {
        snprintf(round_text, sizeof(round_text), "Round %d / %d - Complete", round_count, MAX_PD_ROUNDS);
    } else {
        snprintf(round_text, sizeof(round_text), "Round %d / %d", round_count + 1, MAX_PD_ROUNDS);
    }
    draw_text(r, pd_font, round_text, 40, 80);

    char timer_text[64];
    if (state == PD_STATE_WAITING) {
        snprintf(timer_text, sizeof(timer_text), "Waiting %0.1f...", WAIT_TIME - prompt_timer);
    } else if (state == PD_STATE_ACTIVE) {
        snprintf(timer_text, sizeof(timer_text), "Go! %.1f sec", PROMPT_TIME - prompt_timer);
    } else {
        snprintf(timer_text, sizeof(timer_text), "Finished");
    }
    draw_text(r, pd_font, timer_text, 40, 120);

    if (grid_active) {
        for (int i = 0; i < GRID_ROWS * GRID_COLS; i++) {
            if (!cells[i].visible) continue;
            SDL_Rect rect = { cells[i].x, cells[i].y, CELL_W, CELL_H };
            if (cells[i].target && state != PD_STATE_WAITING) {
                SDL_SetRenderDrawColor(r, 220, 60, 60, 255);
            } else {
                SDL_SetRenderDrawColor(r, 100, 100, 140, 255);
            }
            SDL_RenderFillRect(r, &rect);
            SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
            SDL_RenderDrawRect(r, &rect);

            char label[8];
            snprintf(label, sizeof(label), "%d", cells[i].number);
            draw_text(r, pd_font, label, rect.x + rect.w / 2 - 10, rect.y + rect.h / 2 - 12);
        }
    }
}

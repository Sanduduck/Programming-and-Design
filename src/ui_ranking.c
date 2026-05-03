// 랭킹 화면: 상위 10개 점수 표시

#include <stdio.h>
#include <SDL_ttf.h>
#include "ui_ranking.h"
#include "game_state.h"
#include "types.h"

#define WINDOW_W   1280
#define WINDOW_H   720
#define RANK_COUNT 10

// 컬럼 좌표 (순위 / 점수 / 등급)
#define COL_RANK_X   280
#define COL_RANK_W   120
#define COL_SCORE_X  400
#define COL_SCORE_W  480
#define COL_GRADE_X  880
#define COL_GRADE_W  120
#define ROW_HEIGHT   40

static TTF_Font *title_font = NULL;
static TTF_Font *item_font  = NULL;

// 점수 기록 배열 (score < 0 이면 기록 없음)
static RankEntry ranking[RANK_COUNT] = {
    { -1, "" }, { -1, "" }, { -1, "" }, { -1, "" }, { -1, "" },
    { -1, "" }, { -1, "" }, { -1, "" }, { -1, "" }, { -1, "" }
};

// 폰트 처음 한 번만 로드
static void load_fonts(void) {
    if (!title_font)
        title_font = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 56);
    if (!item_font)
        item_font  = TTF_OpenFont("C:/Windows/Fonts/malgun.ttf", 32);
    if (!title_font || !item_font) {
        printf("폰트 로드 실패: %s\n", TTF_GetError());
    }
}

// 영역 가운데 정렬해서 흰색 텍스트 그리기
static void draw_text_centered(SDL_Renderer *r, TTF_Font *font,
                               const char *text,
                               int box_x, int box_y, int box_w, int box_h) {
    if (!font) return;
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, white);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst = {
        box_x + (box_w - surf->w) / 2,
        box_y + (box_h - surf->h) / 2,
        surf->w, surf->h
    };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void handle_ranking_event(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_RETURN ||
            e->key.keysym.sym == SDLK_m ||
            e->key.keysym.sym == SDLK_ESCAPE) {
            change_state(STATE_MAIN_MENU);
        }
    }
}

void update_ranking(float dt) {
    (void)dt;
}

void draw_ranking(SDL_Renderer *r) {
    load_fonts();

    // 검은 배경
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_Rect bg = { 0, 0, WINDOW_W, WINDOW_H };
    SDL_RenderFillRect(r, &bg);

    // 상단 중앙 제목
    draw_text_centered(r, title_font, "Top 10", 0, 60, WINDOW_W, 70);

    // 컬럼 헤더 (순위 / 점수 / 등급)
    int header_y = 160;
    draw_text_centered(r, item_font, "순위",
                       COL_RANK_X,  header_y, COL_RANK_W,  ROW_HEIGHT);
    draw_text_centered(r, item_font, "점수",
                       COL_SCORE_X, header_y, COL_SCORE_W, ROW_HEIGHT);
    draw_text_centered(r, item_font, "등급",
                       COL_GRADE_X, header_y, COL_GRADE_W, ROW_HEIGHT);

    // 헤더 아래 구분선
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderDrawLine(r, COL_RANK_X, header_y + ROW_HEIGHT + 6,
                       COL_GRADE_X + COL_GRADE_W, header_y + ROW_HEIGHT + 6);

    // 1~10위 표시: [순위] [점수] [등급] 3컬럼
    char rank_str[8], score_str[16], grade_str[8];
    int start_y = 230;
    int gap     = 46;
    for (int i = 0; i < RANK_COUNT; i++) {
        int y = start_y + i * gap;

        snprintf(rank_str, sizeof(rank_str), "%d.", i + 1);

        if (ranking[i].score >= 0) {
            snprintf(score_str, sizeof(score_str), "%d", ranking[i].score);
            snprintf(grade_str, sizeof(grade_str), "%s",
                     ranking[i].grade[0] ? ranking[i].grade : "-");
        } else {
            snprintf(score_str, sizeof(score_str), "-");
            snprintf(grade_str, sizeof(grade_str), "-");
        }

        draw_text_centered(r, item_font, rank_str,
                           COL_RANK_X,  y, COL_RANK_W,  ROW_HEIGHT);
        draw_text_centered(r, item_font, score_str,
                           COL_SCORE_X, y, COL_SCORE_W, ROW_HEIGHT);
        draw_text_centered(r, item_font, grade_str,
                           COL_GRADE_X, y, COL_GRADE_W, ROW_HEIGHT);
    }
}

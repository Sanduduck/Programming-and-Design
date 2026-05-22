#include "proglang.h"
#include "../player.h"
#include <SDL_ttf.h>
#include <string.h>

#define WINDOW_W              1280
#define WINDOW_H              720
#define MAX_LINES             20
#define MAX_BLOCKS_PER_LINE   24
#define LEFT_MARGIN           40
#define FONT_SIZE             40
#define LINE_HEIGHT           40
#define LINE_GAP_V            14
#define RISE_SPEED            100.0f

// 라인 간격(LINE_HEIGHT + LINE_GAP_V)을 RISE_SPEED로 나눠
// "한 라인 분량만큼 올라가는 데 걸리는 시간"을 스폰 간격으로 사용.
// → 화면상에 정확히 LINE_GAP_V 만큼의 빈 공간이 항상 유지됨.
#define SPAWN_INTERVAL        ((float)(LINE_HEIGHT + LINE_GAP_V) / RISE_SPEED)

// 코드 한 줄 내의 '단어 한 덩어리' = 발판 단위.
// 라인 내부 상대 x가 아닌 '화면 절대 x'로 저장 (충돌 판정을 직접 비교하기 위함).
typedef struct {
    int screen_x;   // 블록 좌측 화면 x 좌표
    int width;     // 블록 가로폭 (TTF 측정값)
} Block;

// 화면을 올라가는 코드 한 줄.
typedef struct {
    const char* text;
    Block blocks[MAX_BLOCKS_PER_LINE];
    int block_count;
    float y;
    bool active;
} CodeLine;

// 패턴 데이터: C++ 콘솔 계산기 소스
static const char* calc_code[] = {
    "#include <iostream>",
    "using namespace std;",
    "int main() {",
    "    double a, b;",
    "    char op;",
    "    cin >> a >> op >> b;",
    "    double result = 0;",
    "    switch (op) {",
    "    case '+': result = a + b; break;",
    "    case '-': result = a - b; break;",
    "    case '*': result = a * b; break;",
    "    case '/': result = a / b; break;",
    "    }",
    "    cout << result << endl;",
    "    return 0;",
    "}",
};
#define CALC_LINES (int)(sizeof(calc_code) / sizeof(calc_code[0]))

static CodeLine lines[MAX_LINES];
static float pattern_time;
static int   next_spawn;
static TTF_Font* code_font;

// l->text의 일부 구간[s, s+len)을 임시 버퍼에 복사 후 TTF_SizeUTF8로 측정.
// 단어 블록 위치/폭을 정확히 계산하기 위해 부분 문자열 길이가 필요.
static int seg_width(const char* s, int len) {
    if (len <= 0) return 0;
    char buf[128];
    if (len > (int)sizeof(buf) - 1) len = (int)sizeof(buf) - 1;
    memcpy(buf, s, len);
    buf[len] = '\0';
    int w = 0, h = 0;
    TTF_SizeUTF8(code_font, buf, &w, &h);
    return w;
}

// 라인 텍스트를 토큰 단위 블록 배열로 변환
// 공백/탭은 건너뛰며 x 커서만 전진, 비-공백 구간을 하나의 블록으로 등록.
// 결과적으로 "단어 = 발판", "공백 = 통과 가능 구멍" 이 됨.
static void build_blocks(CodeLine* l) {
    l->block_count = 0;
    int len = (int)strlen(l->text);
    int i = 0;
    int x_cursor = 0;

    while (i < len) {
        // (1) 공백 구간: 위치만 전진, 블록 생성 X
        int ws_start = i;
        while (i < len && (l->text[i] == ' ' || l->text[i] == '\t')) i++;
        if (i > ws_start) x_cursor += seg_width(l->text + ws_start, i - ws_start);
        if (i >= len) break;

        // (2) 토큰 구간: 비-공백 문자 연속 → 한 블록으로 등록
        int tok_start = i;
        while (i < len && l->text[i] != ' ' && l->text[i] != '\t') i++;
        int tw = seg_width(l->text + tok_start, i - tok_start);

        if (l->block_count < MAX_BLOCKS_PER_LINE) {
            l->blocks[l->block_count].screen_x = LEFT_MARGIN + x_cursor;
            l->blocks[l->block_count].width = tw;
            l->block_count++;
        }
        x_cursor += tw;
    }
}

// ----------------------------------------------------------------------------
// 패턴 시작 시 초기화
// ----------------------------------------------------------------------------
// 디스패처(phase1.c)가 패턴 진입 시 1회 호출.
// 폰트는 최초 1회만 로드하여 캐시 (재시작 시에도 재활용).
void proglang_start(void) {
    pattern_time = 0.0f;
    next_spawn = 0;
    for (int i = 0; i < MAX_LINES; i++) lines[i].active = false;
    if (!code_font) code_font = TTF_OpenFont("C:/Windows/Fonts/consola.ttf", FONT_SIZE);
}

// ----------------------------------------------------------------------------
// 라인 슬롯 하나에 calc_code[idx] 줄을 배치
// ----------------------------------------------------------------------------
// 빈 슬롯을 선형 탐색해 첫 자리에 삽입. 초기 y는 화면 아래쪽 바깥(+10px 여유)에서 시작.
static void spawn(int idx) {
    if (idx >= CALC_LINES) return;
    for (int i = 0; i < MAX_LINES; i++) {
        if (!lines[i].active) {
            lines[i].text = calc_code[idx];
            lines[i].y = (float)WINDOW_H + 10.0f;
            lines[i].active = true;
            build_blocks(&lines[i]);
            return;
        }
    }
}

// ----------------------------------------------------------------------------
// 플레이어 중심 x가 특정 블록의 가로 범위 위에 걸쳐 있는지
// ----------------------------------------------------------------------------
// 착지 판정의 첫 단계 — "수평 범위 안인가?" 만 확인.
// 세로 충돌은 호출 측에서 별도 처리.
static bool block_center_over(const Block* b) {
    int px_center = (int)(player.x + PLAYER_W * 0.5f);
    return px_center >= b->screen_x && px_center < b->screen_x + b->width;
}

// ----------------------------------------------------------------------------
// 매 프레임 업데이트
// ----------------------------------------------------------------------------
void proglang_update(float dt) {
    pattern_time += dt;

    // (1) 스폰: 누적 시간이 next_spawn번째 라인의 예약 시점에 도달했으면 스폰.
    //     while 루프로 처리 → dt가 커서 한 프레임에 여러 라인이 밀려도 한꺼번에 따라잡음.
    while (next_spawn < CALC_LINES &&
        (float)next_spawn * SPAWN_INTERVAL <= pattern_time) {
        spawn(next_spawn);
        next_spawn++;
    }

    // (2) 라인 상승 + 화면 위로 완전히 벗어나면 슬롯 해제
    for (int i = 0; i < MAX_LINES; i++) {
        if (!lines[i].active) continue;
        lines[i].y -= RISE_SPEED * dt;
        if (lines[i].y + LINE_HEIGHT < 0) lines[i].active = false;
    }

    // (3) 착지 판정: 하강 중일 때만 검사 (상승 중 머리로 박는 건 통과 허용)
    //     플레이어 발(feet) y 가 어떤 블록의 윗면(top) 부근에 닿으면 그 블록 위에 안착.
    if (player.vy >= 0.0f) {
        float feet = player.y + PLAYER_H;
        for (int i = 0; i < MAX_LINES; i++) {
            if (!lines[i].active) continue;
            for (int j = 0; j < lines[i].block_count; j++) {
                if (!block_center_over(&lines[i].blocks[j])) continue;
                float top = lines[i].y;
                // feet가 라인 세로 범위 [top, top+LINE_HEIGHT] 안에 있을 때만 착지.
                // 라인 두께가 LINE_HEIGHT라 약간의 두께 마진이 있음.
                if (feet < top || feet > top + LINE_HEIGHT) continue;
                player.y = top - PLAYER_H;
                player.vy = 0.0f;
                player.on_ground = true;
                player.jump_count = 0;   // 2단 점프 횟수 리셋
            }
        }
    }

    // (4) 화면 최상단(y=0) 도달 시 피격.
    //     상승하는 라인에 떠밀려 천장에 닿는 즉시 데미지.
    //     무적/깜빡임은 player_damage 내부에서 처리되므로 여기선 별도 플래그 X.
    if (player.y <= 0.0f) {
        player.y = 0.0f;
        player.vy = 0.0f;
        player_damage();
    }
}

// ----------------------------------------------------------------------------
// 패턴 종료 조건
// ----------------------------------------------------------------------------
// 모든 calc_code 줄이 스폰됐고, 그 후 화면에 남은 라인까지 모두 사라졌을 때 종료.
bool proglang_finished(void) {
    if (next_spawn < CALC_LINES) return false;
    for (int i = 0; i < MAX_LINES; i++) if (lines[i].active) return false;
    return true;
}

// ----------------------------------------------------------------------------
// 렌더링
// ----------------------------------------------------------------------------
// 활성 라인을 흰색으로 한 줄씩 TTF 렌더 → 텍스처 생성 → blit → 해제.
// 매 프레임 텍스처를 새로 만드는 단순 구현 (라인 수가 적어 성능 부담 미미).
void proglang_draw(SDL_Renderer* r) {
    SDL_Color white = { 255, 255, 255, 255 };
    for (int i = 0; i < MAX_LINES; i++) {
        if (!lines[i].active) continue;
        SDL_Surface* surf = TTF_RenderUTF8_Blended(code_font, lines[i].text, white);
        if (!surf) continue;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
        // 텍스트를 LINE_HEIGHT 박스 내부에 수직 중앙 정렬.
        SDL_Rect dst = { LEFT_MARGIN, (int)lines[i].y + (LINE_HEIGHT - surf->h) / 2, surf->w, surf->h };
        SDL_RenderCopy(r, tex, NULL, &dst);
        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);
    }
}

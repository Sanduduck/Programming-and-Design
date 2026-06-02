#ifndef PHASE2_H
#define PHASE2_H

// Phase 2 (2학년) 패턴 모듈 디스패처
// 담당 과목: 컴퓨터구조, 데이터통신, 기초전자

#include <SDL.h>
#include <stdbool.h>

typedef enum {
    PHASE2_COMSTR = 0,        // 컴퓨터구조 — 패턴 설계 미정
    PHASE2_DATA,              // 데이터통신 — OSI 7계층 발판 오르기
    PHASE2_ELEC,              // 기초전자 — 패턴 설계 미정
    PHASE2_PDESIGN,           // 프로그래밍설계 — 지시어 셀 이동
    PHASE2_PATTERN_COUNT
} Phase2PatternId;

void phase2_start(Phase2PatternId id);
void phase2_update(float dt);
void phase2_draw(SDL_Renderer *r);
bool phase2_pattern_finished(void);
Phase2PatternId phase2_current(void);

#endif

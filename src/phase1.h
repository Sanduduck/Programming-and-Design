#ifndef PHASE1_H
#define PHASE1_H

// Phase 1 (1학년) 패턴 모듈 디스패처
// 담당 과목: 프로그래밍언어, 공학설계입문, 대학수학, 대학물리학

#include <SDL.h>
#include <stdbool.h>

typedef enum {
    PHASE1_PROGLANG = 0,      // 프로그래밍언어 — 상승 코드 라인 회피
    PHASE1_ENGDESIGN,         // 공학설계입문 — 홈씨어터 빛 회피
    PHASE1_MATH,              // 대학수학 — 퀴즈 정답 선택
    PHASE1_PHYSICS,           // 대학물리학 — 패턴 설계 미정
    PHASE1_PATTERN_COUNT
} Phase1PatternId;

void phase1_start(Phase1PatternId id);
void phase1_update(float dt);
void phase1_draw(SDL_Renderer *r);
bool phase1_pattern_finished(void);
Phase1PatternId phase1_current(void);

#endif

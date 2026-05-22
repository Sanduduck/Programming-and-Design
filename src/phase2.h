#ifndef PHASE2_H
#define PHASE2_H

// Phase 2 (2학년) 패턴 모듈 디스패처
// 담당 과목: 데이터통신

#include <SDL.h>
#include <stdbool.h>

typedef enum {
    PHASE2_DATACOMM = 0,      // 데이터통신 — OSI 7계층 발판 오르기
    PHASE2_PATTERN_COUNT
} Phase2PatternId;

void phase2_start(Phase2PatternId id);
void phase2_update(float dt);
void phase2_draw(SDL_Renderer *r);
bool phase2_pattern_finished(void);
Phase2PatternId phase2_current(void);

#endif

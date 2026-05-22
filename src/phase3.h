#ifndef PHASE3_H
#define PHASE3_H

// Phase 3 (3학년) 패턴 모듈 디스패처
// 담당 과목: 웹서버프로그래밍, 머신러닝, 운영체제, 블록체인

#include <SDL.h>
#include <stdbool.h>

typedef enum {
    PHASE3_BLOCKCHAIN = 0,    // 김도규 - 블록체인
    PHASE3_OS,                // 김도규 - 운영체제
    PHASE3_ML,                // 김인겸 - 머신러닝
    PHASE3_WEBSERVER,         // 정복래 - 웹서버프로그래밍
    PHASE3_PATTERN_COUNT
} Phase3PatternId;

void phase3_start(Phase3PatternId id);
void phase3_update(float dt);
void phase3_draw(SDL_Renderer *r);
bool phase3_pattern_finished(void);
Phase3PatternId phase3_current(void);

#endif

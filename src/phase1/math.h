#ifndef PHASE1_MATH_H
#define PHASE1_MATH_H

// 대학수학 패턴 — 퀴즈(0/1/-1 정답) 표시 후 화면 전환,
// 화면 3분할(좌:-1, 중:0, 우:1)에서 좌우 이동으로 답 선택.
// 제한시간 종료 시 플레이어 위치 기준 답 확정 → 오답이면 HP 차감.

#include <SDL.h>
#include <stdbool.h>

void math_start(void);
void math_update(float dt);
void math_draw(SDL_Renderer *r);
bool math_finished(void);

#endif

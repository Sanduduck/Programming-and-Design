#ifndef PHASE1_ENGDESIGN_H
#define PHASE1_ENGDESIGN_H

// 공학설계입문 패턴 — 홈씨어터 빛 회피
// 천장에 프로젝터/스피커/다운라이트. 광원에서 부채꼴 빛이 발사됨.
// 빛은 WARN(노랑 경고) → ACTIVE(흰빛, 데미지) 2단계.

#include <SDL.h>
#include <stdbool.h>

void engdesign_start(void);
void engdesign_update(float dt);
void engdesign_draw(SDL_Renderer *r);
bool engdesign_finished(void);

#endif

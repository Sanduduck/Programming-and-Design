#ifndef PHASE2_ARCH_H
#define PHASE2_ARCH_H

// 컴퓨터구조 패턴 — TODO: 패턴 설계 확정 후 구현

#include <SDL.h>
#include <stdbool.h>

void arch_start(void);
void arch_update(float dt);
void arch_draw(SDL_Renderer *r);
bool arch_finished(void);

#endif

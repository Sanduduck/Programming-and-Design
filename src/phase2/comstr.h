#ifndef PHASE2_ARCH_H
#define PHASE2_ARCH_H

// 컴퓨터구조 패턴 — TODO: 패턴 설계 확정 후 구현

#include <SDL.h>
#include <stdbool.h>

void comstr_start(void);
void comstr_update(float dt);
void comstr_draw(SDL_Renderer *r);
bool comstr_finished(void);

#endif

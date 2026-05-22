#ifndef PHASE2_ELECTRONICS_H
#define PHASE2_ELECTRONICS_H

// 기초전자 패턴 — TODO: 패턴 설계 확정 후 구현

#include <SDL.h>
#include <stdbool.h>

void electronics_start(void);
void electronics_update(float dt);
void electronics_draw(SDL_Renderer *r);
bool electronics_finished(void);

#endif

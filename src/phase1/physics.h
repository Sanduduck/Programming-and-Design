#ifndef PHASE1_PHYSICS_H
#define PHASE1_PHYSICS_H

// 대학물리학 패턴 — TODO: 패턴 설계 확정 후 구현

#include <SDL.h>
#include <stdbool.h>

void physics_start(void);
void physics_update(float dt);
void physics_draw(SDL_Renderer *r);
bool physics_finished(void);

#endif

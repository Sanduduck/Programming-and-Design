#ifndef PHASE3_ML_H
#define PHASE3_ML_H

// 머신러닝 패턴 (김인겸 교수) — TODO: 패턴 설계 확정 후 구현

#include <SDL.h>
#include <stdbool.h>

void ml_start(void);
void ml_update(float dt);
void ml_draw(SDL_Renderer *r);
bool ml_finished(void);

#endif

#ifndef PHASE2_DATACOMM_H
#define PHASE2_DATACOMM_H

// 데이터통신 패턴 — TODO: 패턴 설계 확정 후 구현

#include <SDL.h>
#include <stdbool.h>

void datacomm_start(void);
void datacomm_update(float dt);
void datacomm_draw(SDL_Renderer *r);
bool datacomm_finished(void);

#endif

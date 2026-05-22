#ifndef PHASE2_DATACOMM_H
#define PHASE2_DATACOMM_H

// 데이터통신 패턴 (Phase 2)
// 좌우로 진동하는 OSI 7계층 발판을 아래(Physical) → 위(Application) 순서로 밟아 오르기

#include <SDL.h>
#include <stdbool.h>

void datacomm_start(void);
void datacomm_update(float dt);
void datacomm_draw(SDL_Renderer *r);
bool datacomm_finished(void);

#endif

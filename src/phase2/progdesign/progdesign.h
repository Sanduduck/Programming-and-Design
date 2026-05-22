#ifndef PHASE2_PROGDESIGN_H
#define PHASE2_PROGDESIGN_H

// 프로그래밍설계 패턴 — TODO: 패턴 설계 확정 후 구현

#include <SDL.h>
#include <stdbool.h>

void progdesign_start(void);
void progdesign_update(float dt);
void progdesign_draw(SDL_Renderer *r);
bool progdesign_finished(void);

#endif

#ifndef PHASE3_OS_H
#define PHASE3_OS_H

// 운영체제 패턴 (김도규 교수) — TODO: 패턴 설계 확정 후 구현

#include <SDL.h>
#include <stdbool.h>

void os_start(void);
void os_update(float dt);
void os_draw(SDL_Renderer *r);
bool os_finished(void);

#endif

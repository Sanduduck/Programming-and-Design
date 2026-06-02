#ifndef PHASE2_DATA_H
#define PHASE2_DATA_H

// 데이터통신 패턴 — OSI 7계층 발판 오르기

#include <SDL.h>
#include <stdbool.h>

void data_start(void);
void data_update(float dt);
void data_draw(SDL_Renderer *r);
bool data_finished(void);

#endif

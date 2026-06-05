#ifndef SIGNAL_H
#define SIGNAL_H

// 이동통신(전파/신호) 패턴 — 좌 → / 우 ← 두 방향에서 전파 막대가 밀려온다.

#include <SDL.h>
#include <stdbool.h>

void signal_start(void);
void signal_update(float dt);
void signal_draw(SDL_Renderer *r);
bool signal_finished(void);

#endif

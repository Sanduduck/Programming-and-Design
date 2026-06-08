#ifndef VIRUS_H
#define VIRUS_H

// 정보보안(바이러스) 패턴 — 좌 → / 우 ← / 위 ↓ 세 방향에서 바이러스가 밀려온다.

#include <SDL.h>
#include <stdbool.h>

void virus_start(void);
void virus_update(float dt);
void virus_draw(SDL_Renderer *r);
bool virus_finished(void);

#endif

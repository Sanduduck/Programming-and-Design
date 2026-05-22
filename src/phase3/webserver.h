#ifndef PHASE3_WEBSERVER_H
#define PHASE3_WEBSERVER_H

// 웹서버프로그래밍 패턴 (정복래 교수) — TODO: 패턴 설계 확정 후 구현

#include <SDL.h>
#include <stdbool.h>

void webserver_start(void);
void webserver_update(float dt);
void webserver_draw(SDL_Renderer *r);
bool webserver_finished(void);

#endif

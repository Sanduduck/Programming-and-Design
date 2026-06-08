#ifndef PHASE3_WEBSERVER_H
#define PHASE3_WEBSERVER_H

// 웹서버프로그래밍 패턴 (정복래 교수) — Node.js URL 낙하 패턴

#include <SDL.h>
#include <stdbool.h>

void webserver_start(void);
void webserver_update(float dt);
void webserver_draw(SDL_Renderer *r);
bool webserver_finished(void);

#endif

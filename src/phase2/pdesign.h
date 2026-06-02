#ifndef PHASE2_PDESIGN_H
#define PHASE2_PDESIGN_H

#include <SDL.h>
#include <stdbool.h>

void pdesign_start(void);
void pdesign_update(float dt);
void pdesign_draw(SDL_Renderer *r);
bool pdesign_finished(void);

#endif // PHASE2_PDESIGN_H

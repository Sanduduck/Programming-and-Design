#ifndef PHASE3_ML_H
#define PHASE3_ML_H

// 머신러닝 패턴 (김인겸 교수)
// 로봇 얼굴이 '허거덩'을 타자기 효과로 생성 → 낙하 → 자모 7조각으로 분해, 3회 반복

#include <SDL.h>
#include <stdbool.h>

void ml_start(void);
void ml_update(float dt);
void ml_draw(SDL_Renderer *r);
bool ml_finished(void);

#endif

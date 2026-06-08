#ifndef PHASE1_PROGLANG_H
#define PHASE1_PROGLANG_H

// 프로그래밍언어 패턴 — Visual Studio C++ 콘솔 계산기 코드가 상승
// 각 라인의 좌측 끝→첫 단어, 마지막 단어→우측 끝 구간은 "벽"(어두운 시안)으로 막힘
// 단어(흰색 텍스트) 사이의 공백만 통과 가능
// 충돌은 모두 1방향 플랫폼 (위에서 착지, 아래에서 통과). 발 중심점 기준
// 폰트 크기는 가장 긴 라인이 화면 가로폭을 채우도록 시작 시 자동 계산
// 플레이어가 화면 천장(y=0)에 닿을 때만 HP 차감

#include <SDL.h>
#include <stdbool.h>

void proglang_start(void);
void proglang_update(float dt);
void proglang_draw(SDL_Renderer *r);
bool proglang_finished(void);

#endif

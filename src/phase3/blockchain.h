#ifndef PHASE3_BLOCKCHAIN_H        // 헤더 중복 포함 방지 (가드 시작)
#define PHASE3_BLOCKCHAIN_H

// 블록체인 패턴 (김도규 교수)
// 하늘에서 크기/타이밍이 서로 다른 비트코인 동전이 낙하

#include <SDL.h>                   // SDL_Renderer 타입 쓰려고 포함
#include <stdbool.h>               // bool 타입 (C99)

void blockchain_start(void);       // 패턴 시작: 타이머/배열 초기화
void blockchain_update(float dt);  // 매 프레임 호출: 스폰/낙하/충돌 처리 (dt = 초)
void blockchain_draw(SDL_Renderer *r); // 매 프레임 호출: 활성 비트코인 그리기
bool blockchain_finished(void);    // 패턴 종료됐는지 확인 (다음 패턴으로 넘길 때 사용)

#endif                             // 헤더 가드 끝

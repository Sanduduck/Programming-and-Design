#ifndef PATTERN_H
#define PATTERN_H

// 패턴 시퀀서 — 학년(Phase)별 과목 패턴을 정해진 순서로 진행한다.
// 현재 패턴이 *_finished() 를 반환하면 다음 패턴을 자동으로 시작.
// 모든 패턴을 통과하면 pattern_finished() 가 true (졸업 클리어).

#include <SDL.h>
#include <stdbool.h>

void pattern_start(void);        // 시퀀스를 처음(Phase1 첫 과목)부터 시작
void pattern_update(float dt);   // 현재 패턴 업데이트 + 끝나면 다음 패턴으로 전환
void pattern_draw(SDL_Renderer *r);
bool pattern_finished(void);     // 전체 시퀀스 완료(졸업) 여부

#endif

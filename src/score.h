#ifndef SCORE_H
#define SCORE_H

#include "types.h"

extern int total_score;

void init_score(void);
void add_score(int amount);
void check_score_for_obstacles(void); // 화면 밖으로 나간 장애물에 점수 부여
void calc_grade(int score, int hp, char *out_grade);

#endif

#ifndef SCORE_H
#define SCORE_H

#include "types.h"

extern int total_score;

void init_score(void);
void add_score(int amount);
void calc_grade(int score, int hp, char *out_grade);

#endif

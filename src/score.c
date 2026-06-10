#define _CRT_SECURE_NO_WARNINGS
#include <string.h>

#include "score.h"

int total_score = 0;

void init_score(void) {
    total_score = 0;
}

void add_score(int amount) {
    total_score += amount;
}

void calc_grade(int score, int hp, char *out_grade) {
    if (hp <= 0) strcpy(out_grade, "F");
    else if (score >= 900) strcpy(out_grade, "A+");
    else if (score >= 750) strcpy(out_grade, "A0");
    else if (score >= 600) strcpy(out_grade, "B+");
    else if (score >= 450) strcpy(out_grade, "B0");
    else if (score >= 300) strcpy(out_grade, "C+");
    else strcpy(out_grade, "C0");
}

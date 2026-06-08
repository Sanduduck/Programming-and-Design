// 점수 누적, 학점 환산

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
    (void)hp;
    // 초당 10점 기준: 220초 생존 시 A+
    if (score >= 2200)      strcpy(out_grade, "A+");
    else if (score >= 1900) strcpy(out_grade, "A0");
    else if (score >= 1600) strcpy(out_grade, "B+");
    else if (score >= 1300) strcpy(out_grade, "B0");
    else if (score >= 1000) strcpy(out_grade, "C+");
    else if (score >= 700)  strcpy(out_grade, "C0");
    else                    strcpy(out_grade, "D0");
}

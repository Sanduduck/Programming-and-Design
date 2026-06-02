// 점수 누적, 학점 환산

#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include "score.h"
#include "obstacle.h"

int total_score = 0;

void init_score(void) {
    total_score = 0;
}

void add_score(int amount) {
    total_score += amount;
}

void check_score_for_obstacles(void) {
    // 화면 왼쪽 밖으로 나간 장애물 중 아직 점수 부여 안된 것에 점수 부여
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;
        if (obstacles[i].scored) continue;
        if (obstacles[i].x + obstacles[i].width < 0 ||
            obstacles[i].x > WINDOW_W) {
            // TODO: 패턴 데이터에서 score_per_obstacle 가져오기
            add_score(10);
            obstacles[i].scored = true;
            obstacles[i].active = false;
        }
    }
}

void calc_grade(int score, int hp, char *out_grade) {
    // HP 0이면 무조건 F
    if (hp <= 0) {
        strcpy(out_grade, "F");
        return;
    }
    // TODO: 실제 점수 분포에 맞춰 기준 조정
    if (score >= 1000)      strcpy(out_grade, "A+");
    else if (score >= 900)  strcpy(out_grade, "A0");
    else if (score >= 800)  strcpy(out_grade, "B+");
    else if (score >= 700)  strcpy(out_grade, "B0");
    else if (score >= 600)  strcpy(out_grade, "C+");
    else if (score >= 500)  strcpy(out_grade, "C0");
    else                    strcpy(out_grade, "D0");
}

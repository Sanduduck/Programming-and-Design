// 패턴 관리: 페이즈별 장애물 패턴 진행

#include <stdbool.h>
#include "pattern.h"
#include "obstacle.h"

int current_phase = 1;
int current_pattern_id = 0;

void init_patterns(void) {
    current_phase = 1;
    current_pattern_id = 0;
    init_obstacles();
}

void start_pattern(int phase, int pattern_id) {
    current_phase = phase;
    current_pattern_id = pattern_id;
    // TODO: 패턴 데이터 로드해서 spawn_obstacle 호출
}

void update_pattern(float dt) {
    // TODO: 시간/조건에 맞춰 장애물 스폰
    (void)dt;
}

bool is_pattern_finished(void) {
    // TODO: 현재 패턴의 모든 장애물이 화면 밖으로 나갔는지 확인
    return false;
}

bool is_phase_finished(void) {
    // TODO: 현재 페이즈의 모든 패턴이 끝났는지 확인
    return false;
}

void next_phase(void) {
    if (current_phase < 4) {
        current_phase++;
        current_pattern_id = 0;
    }
}

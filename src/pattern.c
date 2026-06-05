#include "pattern.h"
#include "obstacle.h"

#define BOX_X 240
#define BOX_Y 120
#define BOX_W 800
#define BOX_H 480

#define VIRUS_PATTERN 0
#define SIGNAL_PATTERN 1
#define VIRUS_OBSTACLE_ID 400
#define SIGNAL_OBSTACLE_ID 410

int current_pattern_id = VIRUS_PATTERN;

static float phase_notice_timer = 0.0f;
static float spawn_timer = 0.0f;
static int wave_count = 0;
static bool pattern_on = false;

void init_patterns(void) {
    current_pattern_id = VIRUS_PATTERN;
    phase_notice_timer = 0.0f;
    spawn_timer = 0.0f;
    wave_count = 0;
    pattern_on = false;
    init_obstacles();
}

void start_pattern(void) {
    current_pattern_id = VIRUS_PATTERN;
    phase_notice_timer = 1.8f;
    spawn_timer = 0.0f;
    wave_count = 0;
    pattern_on = true;
    init_obstacles();
}

static void spawn_fixed_virus_wave(void) {
    spawn_obstacle((float)(BOX_X - 60), 260.0f, 220.0f, 0.0f,
                   42, 42, VIRUS_OBSTACLE_ID);
    spawn_obstacle((float)(BOX_X + BOX_W + 18), 430.0f, -220.0f, 0.0f,
                   42, 42, VIRUS_OBSTACLE_ID);
    spawn_obstacle(640.0f, (float)(BOX_Y - 60), 0.0f, 210.0f,
                   42, 42, VIRUS_OBSTACLE_ID);
}

static void spawn_fixed_signal_wave(void) {
    spawn_obstacle((float)(BOX_X - 90), 285.0f, 230.0f, 0.0f,
                   72, 22, SIGNAL_OBSTACLE_ID);
    spawn_obstacle((float)(BOX_X + BOX_W + 18), 430.0f, -230.0f, 0.0f,
                   72, 22, SIGNAL_OBSTACLE_ID);
}

void update_pattern(float dt) {
    if (!pattern_on) return;

    if (phase_notice_timer > 0.0f) {
        phase_notice_timer -= dt;
        return;
    }

    spawn_timer -= dt;
    if (spawn_timer <= 0.0f) {
        if (current_pattern_id == VIRUS_PATTERN) {
            spawn_fixed_virus_wave();
        } else {
            spawn_fixed_signal_wave();
        }

        wave_count++;
        spawn_timer = 1.1f;
    }

    if (current_pattern_id == VIRUS_PATTERN && wave_count >= 8) {
        current_pattern_id = SIGNAL_PATTERN;
        phase_notice_timer = 1.2f;
        spawn_timer = 0.0f;
        wave_count = 0;
    }

    update_obstacles(dt);
}

bool is_phase_notice_active(void) {
    return phase_notice_timer > 0.0f;
}

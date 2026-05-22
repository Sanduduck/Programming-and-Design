// 운영체제 패턴 (김도규): 애플 로고
// 왼쪽 절반에서 낙하 → 시계방향 90도 회전 → 오른쪽으로 슬라이드
// 베어낸 부분이 안전 통로 (플레이어가 그 자리에 있으면 통과)

#include "os.h"
#include <math.h>
#include <limits.h>

#define WINDOW_W       1280
#define WINDOW_H       720
#define APPLE_RADIUS   300           // 원래: 260 ([120, 640] 좌측 절반에 정확히 들어가는 최대치였음)
#define BITE_R_FRAC    0.50f

// 직립 상태 기준 베어낸 부분 오프셋 (사과 반지름 대비 비율)
#define BITE_OFF_X     0.85f
#define BITE_OFF_Y    -0.32f

// 줄기 자리 노치 (사과 꼭대기에 작은 원으로 둥근 곡선 인덴트 형성)
// 원 중심을 사과 꼭대기 살짝 안쪽에 두어 아래쪽 호가 본체를 깎음
#define NOTCH_OFF_X    0.00f
#define NOTCH_OFF_Y   -0.95f
#define NOTCH_R_FRAC   0.20f

// 잎 (vesica piscis: 두 원의 교집합 → 양 끝이 뾰족한 페탈)
// 로컬 프레임 기준 잎 긴 축은 y축, 두 구성 원의 중심은 x축 위 (±LEAF_D/2, 0)
// LR 키우고 D 도 같이 키우면 긴 축은 늘면서 짧은 축(=LR-D/2)은 줄어 길고 얇아짐
#define LEAF_OFF_X     0.10f
#define LEAF_OFF_Y    -1.10f
#define LEAF_LR_FRAC   0.45f         // 원래: 0.30f
#define LEAF_D_FRAC    0.80f         // 원래: 0.43f
#define LEAF_TILT_DEG  25.0f

// 슬라이드 속도 (player.c MOVE_SPEED 와 동일하게 유지)
#define SLIDE_SPEED    350.0f

// 단계 종료 시각 (초)
#define T_FALL_END     1.8f
#define T_TIP_END      2.5f

// 위치 키 프레임
#define APPLE_START_X  380.0f
#define APPLE_REST_Y   300.0f        // 원래: 340.0f. 사과 바닥(REST_Y + R)이 floor(600)에 닿도록 R 변경에 맞춰 동기 조정

#define PI_F           3.14159265f

static float pattern_time = 0.0f;
static float apple_cx = APPLE_START_X;
static float apple_cy = -APPLE_RADIUS;
static float angle_deg = 0.0f;
static bool  done = false;

static void  rotate_offset(float lx, float ly, float *wx, float *wy);
static float leaf_tip_world_x(void);

void os_start(void) {
    pattern_time = 0.0f;
    apple_cx = APPLE_START_X;
    apple_cy = -(float)APPLE_RADIUS;
    angle_deg = 0.0f;
    done = false;
}

// 단계별 위치/각도 갱신
static void update_motion(float dt) {
    if (pattern_time < T_FALL_END) {
        float t = pattern_time / T_FALL_END;
        float y_start = -(float)APPLE_RADIUS;
        apple_cx = APPLE_START_X;
        apple_cy = y_start + t * (APPLE_REST_Y - y_start);
        angle_deg = 0.0f;
    } else if (pattern_time < T_TIP_END) {
        float t = (pattern_time - T_FALL_END) / (T_TIP_END - T_FALL_END);
        apple_cx = APPLE_START_X;
        apple_cy = APPLE_REST_Y;
        angle_deg = t * 90.0f;
    } else {
        apple_cy = APPLE_REST_Y;
        angle_deg = 90.0f;
        apple_cx += SLIDE_SPEED * dt;
        // 오른쪽으로 누운 사과의 가장 튀어나온 점(잎 끝)이 화면 우측 끝에 닿으면 즉시 사라짐
        if (leaf_tip_world_x() >= (float)WINDOW_W) done = true;
    }
}

// 직립 기준 로컬 오프셋을 현재 각도로 회전 → 월드 좌표
// 화면 좌표계 (y 아래 양수) 에서 시계 방향 회전
static void rotate_offset(float lx, float ly, float *wx, float *wy) {
    float rad = angle_deg * (PI_F / 180.0f);
    float c = cosf(rad);
    float s = sinf(rad);
    *wx = apple_cx + lx * c - ly * s;
    *wy = apple_cy + lx * s + ly * c;
}

// 잎(긴 축의 위쪽 끝)의 월드 x좌표
// vesica piscis 긴 축 반길이 = sqrt(lr² - (d/2)²)
// 잎-로컬 (0, -long_half) 를 (apple_angle + LEAF_TILT) 만큼 회전
static float leaf_tip_world_x(void) {
    float lr = LEAF_LR_FRAC * APPLE_RADIUS;
    float d  = LEAF_D_FRAC  * APPLE_RADIUS;
    float long_half = sqrtf(lr * lr - 0.25f * d * d);

    float leaf_cx, leaf_cy;
    rotate_offset(LEAF_OFF_X * APPLE_RADIUS,
                  LEAF_OFF_Y * APPLE_RADIUS,
                  &leaf_cx, &leaf_cy);

    float leaf_rad = (angle_deg + LEAF_TILT_DEG) * (PI_F / 180.0f);
    return leaf_cx + long_half * sinf(leaf_rad);
}

void os_update(float dt) {
    if (done) return;
    pattern_time += dt;
    update_motion(dt);
}

bool os_finished(void) {
    return done;
}

// 수평 스캔라인으로 원 채우기 (blockchain 모듈과 동일 방식)
static void fill_circle(SDL_Renderer *r, int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)sqrtf((float)(radius * radius - dy * dy));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

// 회전된 vesica piscis (lens) 채우기: 두 원의 교집합
// lr = 각 구성 원의 반지름, d = 두 원의 중심 거리 (< 2*lr)
// 로컬 프레임: 두 원의 중심이 x축 위 (±d/2, 0), 잎의 긴 축은 y축
static void fill_rotated_lens(SDL_Renderer *r,
                               float wcx, float wcy,
                               float lr, float d,
                               float angle_rad) {
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    int bound = (int)(lr) + 2;
    float lr2 = lr * lr;
    float halfd = d * 0.5f;
    int icx = (int)wcx;
    int icy = (int)wcy;

    for (int dy = -bound; dy <= bound; dy++) {
        int run_start = INT_MIN;
        for (int dx = -bound; dx <= bound; dx++) {
            float fx = (float)dx;
            float fy = (float)dy;
            // 월드 (dx, dy) → 잎 로컬 (lx, ly) 역회전
            float lx =  fx * c + fy * s;
            float ly = -fx * s + fy * c;
            float l1x = lx + halfd;
            float l2x = lx - halfd;
            bool inside = (l1x * l1x + ly * ly <= lr2)
                       && (l2x * l2x + ly * ly <= lr2);
            if (inside) {
                if (run_start == INT_MIN) run_start = dx;
            } else if (run_start != INT_MIN) {
                SDL_RenderDrawLine(r,
                                   icx + run_start, icy + dy,
                                   icx + dx - 1,    icy + dy);
                run_start = INT_MIN;
            }
        }
        if (run_start != INT_MIN) {
            SDL_RenderDrawLine(r,
                               icx + run_start, icy + dy,
                               icx + bound,     icy + dy);
        }
    }
}

void os_draw(SDL_Renderer *r) {
    if (done) return;

    float apple_rad = angle_deg * (PI_F / 180.0f);

    // 1) 사과 본체 (흰색 원) — 충돌 박스와 일치
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    fill_circle(r, (int)apple_cx, (int)apple_cy, APPLE_RADIUS);

    // 2) 줄기 자리 노치 (배경색 검정 원, 위치만 사과와 함께 회전)
    float notch_cx, notch_cy;
    rotate_offset(NOTCH_OFF_X * APPLE_RADIUS,
                  NOTCH_OFF_Y * APPLE_RADIUS,
                  &notch_cx, &notch_cy);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    fill_circle(r, (int)notch_cx, (int)notch_cy,
                (int)(NOTCH_R_FRAC * APPLE_RADIUS));

    // 3) 베어낸 부분 (배경색 검정 원)
    float bite_cx, bite_cy;
    rotate_offset(BITE_OFF_X * APPLE_RADIUS,
                  BITE_OFF_Y * APPLE_RADIUS,
                  &bite_cx, &bite_cy);
    fill_circle(r, (int)bite_cx, (int)bite_cy,
                (int)(BITE_R_FRAC * APPLE_RADIUS));

    // 4) 잎 (흰색 vesica piscis, 노치 위로 비스듬히 뻗음)
    float leaf_cx, leaf_cy;
    rotate_offset(LEAF_OFF_X * APPLE_RADIUS,
                  LEAF_OFF_Y * APPLE_RADIUS,
                  &leaf_cx, &leaf_cy);
    float leaf_rad = apple_rad + LEAF_TILT_DEG * (PI_F / 180.0f);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    fill_rotated_lens(r, leaf_cx, leaf_cy,
                      LEAF_LR_FRAC * APPLE_RADIUS,
                      LEAF_D_FRAC * APPLE_RADIUS,
                      leaf_rad);
}

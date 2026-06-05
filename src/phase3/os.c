#include "os.h"
#include "../player.h"
#include <math.h>

#define APPLE_R 200                    // 사과 본체 반지름
#define APPLE_CY_REST (FLOOR_Y - APPLE_R)    // 본체 바닥이 땅에 닿는 높이
#define APPLE_CX0 (APPLE_R + 30)         // 시작 x — 왼쪽 벽에서 30px 띄움
#define FALL_SPEED 450.0f                 // 낙하 속도(px/s)
#define T_ROTATE 1.5f                   // 0→90도 회전 시간(초)
#define SLIDE_SPEED 350.0f                 // player.c MOVE_SPEED 와 동일
#define PI 3.14159265f

// 본체 중심 기준 로컬 오프셋·크기 (APPLE_R 비율) — 깨문 자국 / 잎사귀
#define BITE_OFF_X 0.95f
#define BITE_OFF_Y -0.15f
#define BITE_R_FRAC 0.45f
#define LEAF_OFF_X 0.10f
#define LEAF_OFF_Y -1.12f
#define LEAF_LR_FRAC 0.34f      // 잎 = 두 원의 교집합, 구성 원 반지름
#define LEAF_D_FRAC 0.46f      // 두 원 중심 거리 (작을수록 통통)
#define LEAF_TILT_DEG 30.0f      // 잎 자체 기울기 (오른쪽으로)

// === 안드로이드 단계 ===
#define WAIT_AFTER_APPLE 3.0f                 // 사과 사라진 뒤 대기 시간(초)
#define SWEEP_DURATION 2.0f                 // 180도 반원 스윕에 걸리는 시간(초)
#define ANDROID_CX (WINDOW_W / 2)
#define ANDROID_FEET_Y 600
#define HEAD_CY 444                  // 머리 평평한 밑면(= 중심 y)
#define EYE_LEFT_X (ANDROID_CX - 15)
#define EYE_RIGHT_X (ANDROID_CX + 15)
#define EYE_Y (HEAD_CY - 22)
#define LASER_LEN 2000.0f              // 화면 밖까지 충분히

static float apple_cx;
static float apple_cy;
static float angle_deg;    // 0 → 90 (시계방향)
static float post_t;       // 사과 사라진 후 경과 시간 (-1이면 아직 사과 단계)
static bool  done;

void os_start(void) {
    apple_cx = APPLE_CX0;
    apple_cy = -(float)APPLE_R;   // 화면 위에서 시작
    angle_deg = 0.0f;
    post_t = -1.0f;
    done = false;
}

// 로컬 오프셋(lx,ly)을 angle_deg만큼 시계방향 회전 → 월드 좌표
static void rotate_pt(float lx, float ly, float *wx, float *wy) {
    float rad = angle_deg * (PI / 180.0f);
    float c = cosf(rad), s = sinf(rad);
    *wx = apple_cx + lx * c - ly * s;
    *wy = apple_cy + lx * s + ly * c;
}

// 광선(원점 ox,oy, 단위 방향 dx,dy)이 플레이어 중심 근처를 지나가나
static bool laser_hits_player(float ox, float oy, float dx, float dy) {
    float px = player.x + PLAYER_W / 2.0f;
    float py = player.y + PLAYER_H / 2.0f;
    float vx = px - ox, vy = py - oy;
    if (vx * dx + vy * dy < 0.0f) return false;     // 광선 뒤쪽
    return fabsf(vx * dy - vy * dx) <= PLAYER_W / 2.0f;
}

void os_update(float dt) {
    if (done) return;

    if (post_t < 0.0f) {
        // 사과 떨어지는 단계
        if (apple_cy < APPLE_CY_REST) {  // 낙하
            apple_cy += FALL_SPEED * dt;
        } else if (angle_deg < 90.0f) {   // 회전
            angle_deg += 90.0f / T_ROTATE * dt;
        } else {  // 슬라이드
            apple_cx += SLIDE_SPEED * dt;
        }

        // 충돌: 본체 원 안 && 깨문 자국 원 밖 = 사과 솔리드. 플레이어 사각 5점 샘플
        float bx, by;
        rotate_pt(BITE_OFF_X * APPLE_R, BITE_OFF_Y * APPLE_R, &bx, &by);
        float br = BITE_R_FRAC * APPLE_R;
        float sx[5] = { player.x, player.x + PLAYER_W, player.x,
                        player.x + PLAYER_W, player.x + PLAYER_W / 2.0f };
        float sy[5] = { player.y, player.y, player.y + PLAYER_H,
                        player.y + PLAYER_H, player.y + PLAYER_H / 2.0f };
        for (int i = 0; i < 5; i++) {
            float bdx = sx[i] - apple_cx, bdy = sy[i] - apple_cy;
            float ndx = sx[i] - bx, ndy = sy[i] - by;
            if (bdx * bdx + bdy * bdy <= (float)APPLE_R * APPLE_R &&
                ndx * ndx + ndy * ndy >  br * br) {
                player_damage();
                break;
            }
        }

        // 잎사귀가 화면 오른쪽 벽에 닿으면 사과 사라짐 → 안드로이드 대기 시작
        float lx, ly;
        rotate_pt(LEAF_OFF_X * APPLE_R, LEAF_OFF_Y * APPLE_R, &lx, &ly);
        if (lx + LEAF_LR_FRAC * APPLE_R >= WINDOW_W) post_t = 0.0f;
    } else {
        // 안드로이드 단계
        post_t += dt;
        float sweep_t = post_t - WAIT_AFTER_APPLE;
        if (sweep_t < 0.0f) return;                    // 대기 중
        if (sweep_t >= SWEEP_DURATION) { done = true; return; }

        // θ: 180°(왼쪽) → 90°(아래) → 0°(오른쪽) 로 시계방향 반원 스윕
        float theta_rad = (180.0f - sweep_t / SWEEP_DURATION * 180.0f) * (PI / 180.0f);
        float dx = cosf(theta_rad), dy = sinf(theta_rad);
        if (laser_hits_player(EYE_LEFT_X,  EYE_Y, dx, dy) || laser_hits_player(EYE_RIGHT_X, EYE_Y, dx, dy)) {
            player_damage();
        }
    }
}

bool os_finished(void) {
    return done;
}

// 수평 스캔라인으로 원 채움
static void fill_circle(SDL_Renderer *r, int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)sqrtf((float)(radius * radius - dy * dy));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

// 회전된 vesica piscis(두 원 교집합) = 양 끝이 뾰족한 잎사귀 모양 채움
static void fill_leaf(SDL_Renderer *r, float cx, float cy,
                      float lr, float d, float rot_deg) {
    float rad = rot_deg * (PI / 180.0f);
    float c = cosf(rad), s = sinf(rad);
    float hd = d / 2.0f;
    int bound = (int)lr + 2;
    for (int dy = -bound; dy <= bound; dy++) {
        for (int dx = -bound; dx <= bound; dx++) {
            float lx =  dx * c + dy * s;          // 월드 → 잎 로컬 역회전
            float ly = -dx * s + dy * c;
            float e1 = lx + hd, e2 = lx - hd;     // 두 원 중심 (±d/2, 0)
            if (e1 * e1 + ly * ly <= lr * lr &&
                e2 * e2 + ly * ly <= lr * lr)
                SDL_RenderDrawPoint(r, (int)cx + dx, (int)cy + dy);
        }
    }
}

// 안드로이드: 다리 / 몸통 / 팔 / 머리(반원) / 안테나 / 눈
static void draw_android(SDL_Renderer *r) {
    SDL_SetRenderDrawColor(r, 164, 199, 57, 255);    // 안드로이드 녹색

    SDL_Rect leg_l = { ANDROID_CX - 23, ANDROID_FEET_Y - 30, 16, 30 };
    SDL_Rect leg_r = { ANDROID_CX + 7,  ANDROID_FEET_Y - 30, 16, 30 };
    SDL_Rect body  = { ANDROID_CX - 50, ANDROID_FEET_Y - 30 - 120, 100, 120 };
    SDL_Rect arm_l = { ANDROID_CX - 71, body.y + 12, 18, 90 };
    SDL_Rect arm_r = { ANDROID_CX + 53, body.y + 12, 18, 90 };
    SDL_RenderFillRect(r, &leg_l);
    SDL_RenderFillRect(r, &leg_r);
    SDL_RenderFillRect(r, &body);
    SDL_RenderFillRect(r, &arm_l);
    SDL_RenderFillRect(r, &arm_r);

    // 머리: 위쪽 반원만 (dy = -hr..0 → 평평한 밑면)
    int hr = 45;
    for (int dy = -hr; dy <= 0; dy++) {
        int dx = (int)sqrtf((float)(hr * hr - dy * dy));
        SDL_RenderDrawLine(r, ANDROID_CX - dx, HEAD_CY + dy,
                              ANDROID_CX + dx, HEAD_CY + dy);
    }

    // 안테나 2개 (머리 위로 비스듬히)
    SDL_RenderDrawLine(r, ANDROID_CX - 18, HEAD_CY - 40, ANDROID_CX - 28, HEAD_CY - 58);
    SDL_RenderDrawLine(r, ANDROID_CX + 18, HEAD_CY - 40, ANDROID_CX + 28, HEAD_CY - 58);

    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);   // 눈 흰색
    fill_circle(r, EYE_LEFT_X,  EYE_Y, 5);
    fill_circle(r, EYE_RIGHT_X, EYE_Y, 5);
}

// 광선 한 줄 — 원점에서 (dx,dy) 방향으로 화면 밖까지
static void draw_laser(SDL_Renderer *r, float ox, float oy, float dx, float dy) {
    SDL_SetRenderDrawColor(r, 255, 80, 80, 255);
    SDL_RenderDrawLine(r, (int)ox, (int)oy,
        (int)(ox + dx * LASER_LEN),
        (int)(oy + dy * LASER_LEN));
}

void os_draw(SDL_Renderer *r) {
    if (done) return;

    if (post_t < 0.0f) {
        // === 사과 그리기 ===
        float bx, by, lx, ly;
        rotate_pt(BITE_OFF_X * APPLE_R, BITE_OFF_Y * APPLE_R, &bx, &by);
        rotate_pt(LEAF_OFF_X * APPLE_R, LEAF_OFF_Y * APPLE_R, &lx, &ly);

        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        fill_circle(r, (int)apple_cx, (int)apple_cy, APPLE_R);          // 본체
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);                        // 배경색으로 깨문 자국
        fill_circle(r, (int)bx, (int)by, (int)(BITE_R_FRAC * APPLE_R));
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);                  // 잎사귀
        fill_leaf(r, lx, ly, LEAF_LR_FRAC * APPLE_R, LEAF_D_FRAC * APPLE_R,
                  angle_deg + LEAF_TILT_DEG);
    } else {
        float sweep_t = post_t - WAIT_AFTER_APPLE;
        if (sweep_t < 0.0f) return;     // 대기 — 화면 비움

        draw_android(r);

        float theta_rad = (180.0f - sweep_t / SWEEP_DURATION * 180.0f) * (PI / 180.0f);
        float dx = cosf(theta_rad), dy = sinf(theta_rad);
        draw_laser(r, EYE_LEFT_X,  EYE_Y, dx, dy);
        draw_laser(r, EYE_RIGHT_X, EYE_Y, dx, dy);
    }
}
// 패턴 시퀀서 구현 — 과목 패턴 모듈을 함수 포인터 테이블로 묶어 순서대로 진행.
// 호출 순서(팀 확정):
//   Phase1: 프로그래밍언어 → 대학물리학 → 대학수학 → 공학설계입문
//   Phase2: 데이터통신 → 기초전자 → 컴퓨터구조 → 프로그래밍설계
//   Phase3: 블록체인 → 운영체제 → 머신러닝 → 웹서버프로그래밍
//   Phase4: 정보보안 → 이동통신

#include "pattern.h"

#include "phase1/proglang.h"
#include "phase1/physics.h"
#include "phase1/math.h"
#include "phase1/engdesign.h"
#include "phase2/data.h"
#include "phase2/elec.h"
#include "phase2/comstr.h"
#include "phase2/pdesign.h"
#include "phase3/blockchain.h"
#include "phase3/os.h"
#include "phase3/ml.h"
#include "phase3/webserver.h"
#include "phase4/virus.h"
#include "phase4/signal.h"
#include "player.h"
#include "score.h"

typedef struct {
    void (*start)(void);
    void (*update)(float dt);
    void (*draw)(SDL_Renderer *r);
    bool (*finished)(void);
} PatternStep;

static const PatternStep steps[] = {
    // Phase 1
    { proglang_start,   proglang_update,   proglang_draw,   proglang_finished   },
    { physics_start,    physics_update,    physics_draw,    physics_finished    },
    { math_start,       math_update,       math_draw,       math_finished       },
    { engdesign_start,  engdesign_update,  engdesign_draw,  engdesign_finished  },
    // Phase 2
    { data_start,       data_update,       data_draw,       data_finished       },
    { elec_start,       elec_update,       elec_draw,       elec_finished       },
    { comstr_start,     comstr_update,     comstr_draw,     comstr_finished     },
    { pdesign_start,    pdesign_update,    pdesign_draw,    pdesign_finished    },
    // Phase 3
    { blockchain_start, blockchain_update, blockchain_draw, blockchain_finished },
    { os_start,         os_update,         os_draw,         os_finished         },
    { ml_start,         ml_update,         ml_draw,         ml_finished         },
    { webserver_start,  webserver_update,  webserver_draw,  webserver_finished  },
    // Phase 4
    { virus_start,      virus_update,      virus_draw,      virus_finished      },
    { signal_start,     signal_update,     signal_draw,     signal_finished     },
};

#define STEP_COUNT ((int)(sizeof(steps) / sizeof(steps[0])))

static int current;

void pattern_start(void) {
    current = 0;
    init_score();
    steps[0].start();
}

void pattern_update(float dt) {
    if (current >= STEP_COUNT) return;

    steps[current].update(dt);

    if (steps[current].finished()) {
        current++;
        if (current < STEP_COUNT) {
            // 각 Phase는 4개 패턴으로 구성된다. 새 Phase 시작 시 HP 회복.
            if (current % 4 == 0) player.hp = 4;
            steps[current].start();
        }
    }
}

void pattern_draw(SDL_Renderer *r) {
    if (current < STEP_COUNT) steps[current].draw(r);
}

bool pattern_finished(void) {
    return current >= STEP_COUNT;
}

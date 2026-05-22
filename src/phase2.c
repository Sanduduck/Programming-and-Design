// Phase 2 패턴 디스패처
// 활성 패턴 ID에 따라 phase2/ 하위 모듈로 위임

#include "phase2.h"
#include "phase2/datacomm.h"

static Phase2PatternId active = PHASE2_DATACOMM;

void phase2_start(Phase2PatternId id) {
    active = id;
    switch (id) {
        case PHASE2_DATACOMM: datacomm_start(); break;
        default: break;
    }
}

void phase2_update(float dt) {
    switch (active) {
        case PHASE2_DATACOMM: datacomm_update(dt); break;
        default: break;
    }
}

void phase2_draw(SDL_Renderer *r) {
    switch (active) {
        case PHASE2_DATACOMM: datacomm_draw(r); break;
        default: break;
    }
}

bool phase2_pattern_finished(void) {
    switch (active) {
        case PHASE2_DATACOMM: return datacomm_finished();
        default: return true;
    }
}

Phase2PatternId phase2_current(void) {
    return active;
}

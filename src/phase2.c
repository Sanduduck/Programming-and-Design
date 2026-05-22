// Phase 2 패턴 디스패처
// 활성 패턴 ID에 따라 phase2/ 하위 모듈로 위임

#include "phase2.h"
#include "phase2/datacomm/datacomm.h"
#include "phase2/progdesign/progdesign.h"
#include "phase2/arch/arch.h"
#include "phase2/electronics/electronics.h"

static Phase2PatternId active = PHASE2_DATACOMM;

void phase2_start(Phase2PatternId id) {
    active = id;
    switch (id) {
        case PHASE2_DATACOMM:    datacomm_start();    break;
        case PHASE2_PROGDESIGN:  progdesign_start();  break;
        case PHASE2_ARCH:        arch_start();        break;
        case PHASE2_ELECTRONICS: electronics_start(); break;
        default: break;
    }
}

void phase2_update(float dt) {
    switch (active) {
        case PHASE2_DATACOMM:    datacomm_update(dt);    break;
        case PHASE2_PROGDESIGN:  progdesign_update(dt);  break;
        case PHASE2_ARCH:        arch_update(dt);        break;
        case PHASE2_ELECTRONICS: electronics_update(dt); break;
        default: break;
    }
}

void phase2_draw(SDL_Renderer *r) {
    switch (active) {
        case PHASE2_DATACOMM:    datacomm_draw(r);    break;
        case PHASE2_PROGDESIGN:  progdesign_draw(r);  break;
        case PHASE2_ARCH:        arch_draw(r);        break;
        case PHASE2_ELECTRONICS: electronics_draw(r); break;
        default: break;
    }
}

bool phase2_pattern_finished(void) {
    switch (active) {
        case PHASE2_DATACOMM:    return datacomm_finished();
        case PHASE2_PROGDESIGN:  return progdesign_finished();
        case PHASE2_ARCH:        return arch_finished();
        case PHASE2_ELECTRONICS: return electronics_finished();
        default: return true;
    }
}

Phase2PatternId phase2_current(void) {
    return active;
}

// Phase 2 패턴 디스패처
// 활성 패턴 ID에 따라 phase2/ 하위 모듈로 위임

#include "phase2.h"
#include "phase2/comstr.h"
#include "phase2/data.h"
#include "phase2/elec.h"
#include "phase2/pdesign.h"

static Phase2PatternId active = PHASE2_COMSTR;

void phase2_start(Phase2PatternId id) {
    active = id;
    switch (id) {
        case PHASE2_COMSTR:  comstr_start();  break;
        case PHASE2_DATA:    data_start();    break;
        case PHASE2_ELEC:    elec_start();    break;
        case PHASE2_PDESIGN: pdesign_start(); break;
        default: break;
    }
}

void phase2_update(float dt) {
    switch (active) {
        case PHASE2_COMSTR:  comstr_update(dt);  break;
        case PHASE2_DATA:    data_update(dt);    break;
        case PHASE2_ELEC:    elec_update(dt);    break;
        case PHASE2_PDESIGN: pdesign_update(dt); break;
        default: break;
    }
}

void phase2_draw(SDL_Renderer *r) {
    switch (active) {
        case PHASE2_COMSTR:  comstr_draw(r);  break;
        case PHASE2_DATA:    data_draw(r);    break;
        case PHASE2_ELEC:    elec_draw(r);    break;
        case PHASE2_PDESIGN: pdesign_draw(r); break;
        default: break;
    }
}

bool phase2_pattern_finished(void) {
    switch (active) {
        case PHASE2_COMSTR:  return comstr_finished();
        case PHASE2_DATA:    return data_finished();
        case PHASE2_ELEC:    return elec_finished();
        case PHASE2_PDESIGN: return pdesign_finished();
        default: return true;
    }
}

Phase2PatternId phase2_current(void) {
    return active;
}

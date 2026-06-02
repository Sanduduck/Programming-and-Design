// Phase 1 패턴 디스패처
// 활성 패턴 ID에 따라 phase1/ 하위 모듈로 위임

#include "phase1.h"
#include "phase1/proglang.h"
#include "phase1/engdesign.h"
#include "phase1/math.h"
#include "phase1/physics.h"

static Phase1PatternId active = PHASE1_PROGLANG;

void phase1_start(Phase1PatternId id) {
    active = id;
    switch (id) {
        case PHASE1_PROGLANG:  proglang_start();  break;
        case PHASE1_ENGDESIGN: engdesign_start(); break;
        case PHASE1_MATH:      math_start();      break;
        case PHASE1_PHYSICS:   physics_start();   break;
        default: break;
    }
}

void phase1_update(float dt) {
    switch (active) {
        case PHASE1_PROGLANG:  proglang_update(dt);  break;
        case PHASE1_ENGDESIGN: engdesign_update(dt); break;
        case PHASE1_MATH:      math_update(dt);      break;
        case PHASE1_PHYSICS:   physics_update(dt);   break;
        default: break;
    }
}

void phase1_draw(SDL_Renderer *r) {
    switch (active) {
        case PHASE1_PROGLANG:  proglang_draw(r);  break;
        case PHASE1_ENGDESIGN: engdesign_draw(r); break;
        case PHASE1_MATH:      math_draw(r);      break;
        case PHASE1_PHYSICS:   physics_draw(r);   break;
        default: break;
    }
}

bool phase1_pattern_finished(void) {
    switch (active) {
        case PHASE1_PROGLANG:  return proglang_finished();
        case PHASE1_ENGDESIGN: return engdesign_finished();
        case PHASE1_MATH:      return math_finished();
        case PHASE1_PHYSICS:   return physics_finished();
        default: return true;
    }
}

Phase1PatternId phase1_current(void) {
    return active;
}

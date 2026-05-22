// Phase 1 패턴 디스패처
// 활성 패턴 ID에 따라 phase1/ 하위 모듈로 위임

#include "phase1.h"
#include "phase1/physics.h"
#include "phase1/math.h"
#include "phase1/proglang.h"
#include "phase1/engdesign.h"

typedef struct {
    void (*start)(void);
    void (*update)(float);
    void (*draw)(SDL_Renderer *);
    bool (*finished)(void);
} PatternVT;

static const PatternVT vt[PHASE1_PATTERN_COUNT] = {
    { physics_start,   physics_update,   physics_draw,   physics_finished   },
    { math_start,      math_update,      math_draw,      math_finished      },
    { proglang_start,  proglang_update,  proglang_draw,  proglang_finished  },
    { engdesign_start, engdesign_update, engdesign_draw, engdesign_finished },
};

static Phase1PatternId active = PHASE1_PROGLANG;

void phase1_start(Phase1PatternId id) { active = id; vt[id].start(); }
void phase1_update(float dt)          { vt[active].update(dt); }
void phase1_draw(SDL_Renderer *r)     { vt[active].draw(r); }
bool phase1_pattern_finished(void)    { return vt[active].finished(); }
Phase1PatternId phase1_current(void)  { return active; }

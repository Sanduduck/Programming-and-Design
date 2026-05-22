// Phase 3 패턴 디스패처
// 활성 패턴 ID에 따라 phase3/ 하위 모듈로 위임

#include "phase3.h"
#include "phase3/blockchain.h"
#include "phase3/os.h"
#include "phase3/ml.h"
#include "phase3/webserver.h"

static Phase3PatternId active = PHASE3_BLOCKCHAIN;

void phase3_start(Phase3PatternId id) {
    active = id;
    switch (id) {
        case PHASE3_BLOCKCHAIN: blockchain_start(); break;
        case PHASE3_OS:         os_start();         break;
        case PHASE3_ML:         ml_start();         break;
        case PHASE3_WEBSERVER:  webserver_start();  break;
        default: break;
    }
}

void phase3_update(float dt) {
    switch (active) {
        case PHASE3_BLOCKCHAIN: blockchain_update(dt); break;
        case PHASE3_OS:         os_update(dt);         break;
        case PHASE3_ML:         ml_update(dt);         break;
        case PHASE3_WEBSERVER:  webserver_update(dt);  break;
        default: break;
    }
}

void phase3_draw(SDL_Renderer *r) {
    switch (active) {
        case PHASE3_BLOCKCHAIN: blockchain_draw(r); break;
        case PHASE3_OS:         os_draw(r);         break;
        case PHASE3_ML:         ml_draw(r);         break;
        case PHASE3_WEBSERVER:  webserver_draw(r);  break;
        default: break;
    }
}

bool phase3_pattern_finished(void) {
    switch (active) {
        case PHASE3_BLOCKCHAIN: return blockchain_finished();
        case PHASE3_OS:         return os_finished();
        case PHASE3_ML:         return ml_finished();
        case PHASE3_WEBSERVER:  return webserver_finished();
        default: return true;
    }
}

Phase3PatternId phase3_current(void) {
    return active;
}

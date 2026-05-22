#ifndef SETTINGS_H
#define SETTINGS_H

#include "types.h"

// 전역 설정 (다른 모듈에서 참조)
extern Settings g_settings;

void init_settings(void);
void settings_set_scheme(ControlScheme scheme);

// 마스터 볼륨 반영한 실효 볼륨 (0~100)
int  settings_effective_bgm(void);
int  settings_effective_sfx(void);

#endif

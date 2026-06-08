#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

#include <stdbool.h>

bool audio_init(void);
void audio_shutdown(void);
void audio_play_jump(void);

#endif

#ifndef PATTERN_H
#define PATTERN_H

#include "types.h"

extern int current_phase;
extern int current_pattern_id;

void init_patterns(void);
void start_pattern(int phase, int pattern_id);
void update_pattern(float dt);
bool is_pattern_finished(void);
bool is_phase_finished(void);
void next_phase(void);

#endif

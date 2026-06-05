#include "types.h"

extern int current_pattern_id;

void init_patterns(void);
void start_pattern(void);
void update_pattern(float dt);
bool is_phase_notice_active(void);

#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include "types.h"

bool check_collision(const Player *p, const Obstacle *o);
void check_all_collisions(void);

#endif

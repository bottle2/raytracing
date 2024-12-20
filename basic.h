#ifndef BASIC_H
#define BASIC_H

#include "precision.h"

static inline dist basic_sum(dist lhs, dist rhs) { return lhs + rhs; }
static inline dist basic_sub(dist lhs, dist rhs) { return lhs - rhs; }
static inline dist basic_div(dist lhs, dist rhs) { return lhs / rhs; }
static inline dist basic_mul(dist lhs, dist rhs) { return lhs * rhs; }

#endif

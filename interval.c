// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include <math.h>
#include <stdbool.h>

#include "interval.h"

bool interval_contains(struct interval interval, dist x)
{
    return interval.min <= x && x <= interval.max;
}

bool interval_surrounds(struct interval interval, dist x)
{
    return interval.min < x && x < interval.max;
}

dist interval_clamp(struct interval interval, dist x)
{
    if (x < interval.min) return interval.min;
    if (x > interval.max) return interval.max;
    return x;
}

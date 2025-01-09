// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#ifndef INTERVAL_H
#define INTERVAL_H

#include <stdbool.h>

#include "precision.h"

struct interval
{
    dist min;
    dist max;
};

bool interval_contains (struct interval interval, dist x);
bool interval_surrounds(struct interval interval, dist x);
dist interval_clamp    (struct interval interval, dist x);

#define INTERVAL(...) (struct interval){__VA_ARGS__}

#define INTERVAL_EMPTY    INTERVAL(+HUGE_VAL,-HUGE_VAL)
#define INTERVAL_UNIVERSE INTERVAL(-HUGE_VAL,+HUGE_VAL)

#endif

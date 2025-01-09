// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#ifndef COLOR_H
#define COLOR_H

#include <tgmath.h>

#include "interval.h"
#include "vec3.h"

#define COLOR(...) VEC3(__VA_ARGS__)
#define COLOR_C(...) VEC3_C(__VA_ARGS__)
#define COLOR_L(...) VEC3_L(__VA_ARGS__)
#define COLOR_FMT "%d %d %d"
#define COLOR_BLOW(IT, SPP) \
    (int)(255.999 * interval_clamp(intensity, sqrt((IT).r / SPP))), \
    (int)(255.999 * interval_clamp(intensity, sqrt((IT).g / SPP))), \
    (int)(255.999 * interval_clamp(intensity, sqrt((IT).b / SPP)))

static struct interval const intensity = {0, 0.999};

#endif

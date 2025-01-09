// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#ifndef HIT_RECORD
#define HIT_RECORD

#include <stdbool.h>

#include "vec3.h"
#include "ray.h"

struct hit_record
{
    union vec3 p;
    union vec3 normal;
    struct material *mat;
    dist t;
    bool front_face;
};

void hit_record_set_face_normal(
    struct hit_record *hit,
    struct ray         ray,
    union vec3 outward_normal
);

#endif

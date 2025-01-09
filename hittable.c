// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include <stdbool.h>

#include "hittable.h"
#include "interval.h"
#include "sphere.h"

bool hittable_hit(
    struct hittable   *hittable,
    struct ray         ray,
    struct interval    ray_t,
    struct hit_record *rec
) {
    struct hit_record temp_rec;
    bool hit_anything = false;
    dist closest_so_far = ray_t.max;

    #define X(IT) for (int i = 0; i < hittable->n_##IT; i++) \
        if (IT##_hit(hittable->IT##s + i, ray, INTERVAL(ray_t.min, closest_so_far), &temp_rec)) \
        { \
            hit_anything = true; \
            closest_so_far = temp_rec.t; \
            *rec = temp_rec; \
        }

    HITTABLE_XS

    #undef X

    return hit_anything;
}

#ifndef HITTABLE_H
#define HITTABLE_H

#include "hit_record.h"
#include "interval.h"
#include "ray.h"

#define HITTABLE_XS X(sphere) X(hittable)

#define X(IT) struct IT *IT##s; int n_##IT;

struct hittable { HITTABLE_XS };

#undef X

bool hittable_hit(
    struct hittable   *hittable,
    struct ray         ray,
    struct interval    ray_t,
    struct hit_record *rec
);

#endif

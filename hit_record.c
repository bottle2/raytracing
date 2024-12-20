#include <assert.h>
#include <tgmath.h>

#include "hittable.h"

void hit_record_set_face_normal(
    struct hit_record *hit,
    struct ray         ray,
    union vec3 outward_normal
) {
    assert(fabs(len(outward_normal) - 1) < 0.0001); // XXX
    hit->front_face = dot(ray.dir, outward_normal) < 0;
    hit->normal = hit->front_face ? outward_normal : neg(outward_normal);
}

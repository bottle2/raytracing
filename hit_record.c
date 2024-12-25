#include <assert.h>
#include <tgmath.h>

#include "hittable.h"

void hit_record_set_face_normal(
    struct hit_record *hit,
    struct ray         ray,
    union vec3 outward_normal
) {
#if 0
    assert(fabs(len(outward_normal) - 1) < 0.0001); // XXX
#endif // TODO maybe we can use strong typing and have a uvec3
       // which can only be constructed from certain ways.
       // Crashing when precision is float, double is okay
    hit->front_face = dot(ray.dir, outward_normal) < 0;
    hit->normal = hit->front_face ? outward_normal : neg(outward_normal);
}

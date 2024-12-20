#include <stdbool.h>

#include "material.h"
#include "vec3.h"
#include "util.h"

bool material_scatter_lambertian(
    struct material *mat,
    struct ray r_in,
    struct hit_record rec,
    union vec3 *attenuation,
    struct ray *scattered
) {
    (void)r_in;
    union vec3 scatter_direction = sum(rec.normal, vec3_random_unit_vector());

    if (vec3_near_zero(scatter_direction))
        scatter_direction = rec.normal;

    *scattered = RAY(rec.p, scatter_direction);
    *attenuation = mat->albedo;
    return true;
}

bool material_scatter_metal(
    struct material *mat,
    struct ray r_in,
    struct hit_record rec,
    union vec3 *attenuation,
    struct ray *scattered
) {
    union vec3 reflected = vec3_reflect(unit(r_in.dir), rec.normal);
    *scattered = RAY(rec.p, sum(reflected, mul(mat->fuzz, vec3_random_in_unit_sphere())));
    *attenuation = mat->albedo;
    return dot(scattered->dir, rec.normal) > 0;
}

static dist reflectance(dist cosine, dist ref_idx)
{
    // Use Schlick's approximation for reflectance.
    dist r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

bool material_scatter_dielectric(
    struct material *mat,
    struct ray r_in,
    struct hit_record rec,
    union vec3 *attenuation,
    struct ray *scattered
) {
    *attenuation = COLOR(1, 1, 1);
    dist refraction_ratio = rec.front_face ? 1 / mat->ir : mat->ir;
    union vec3 unit_direction = unit(r_in.dir);

    dist cos_theta = fmin(dot(neg(unit_direction), rec.normal), 1);
    dist sin_theta = sqrt(1 - cos_theta * cos_theta);

    bool cannot_refract = refraction_ratio * sin_theta > 1;
    union vec3 direction = cannot_refract || reflectance(cos_theta, refraction_ratio) > random01()
                         ? vec3_reflect(unit_direction, rec.normal)
                         : vec3_refract(unit_direction, rec.normal, refraction_ratio);

    *scattered = RAY(rec.p, direction);
    return true;
}

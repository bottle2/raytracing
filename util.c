#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "precision.h"
#include "util.h"

dist util_deg2rad(dist deg)
{
    return deg * M_PI / 180.0;
}

dist util_rad2deg(dist rad)
{
    return rad * 180.0 / M_PI;
}

// Returns a random real in [0,1). 
dist random01(void)
{
    return rand() / (RAND_MAX + 1.0);
}

// Returns a random real in [min,max). 
dist random_interval(dist min, dist max)
{
    return min + (max - min) * random01();
}

#ifndef UTIL_H
#define UTIL_H

#include "precision.h"

dist util_deg2rad(dist deg);

// Returns a random real in [0,1). 
dist random01(void);

// Returns a random real in [min,max). 
dist random_interval(dist min, dist max);

#define UNCHECKED(...) (__VA_ARGS__)
#define NIL

#define TRY(IT) if ((IT)) { \
    SDL_LogError( \
        SDL_LOG_CATEGORY_ERROR, \
        __FILE__ ":%d: %s\n", __LINE__, SDL_GetError() \
    ); exit(EXIT_FAILURE); } else (void)0

#endif

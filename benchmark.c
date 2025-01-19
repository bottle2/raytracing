// SPDX-FileCopyrightText: Copyright (c) 2025 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <SDL.h>

#include "benchmark.h"

void (*benchmark_reset)(void) = NULL;

#define X(L, S) BENCHMARK_EVENT_##L,
enum benchmark_event { BENCHMARK_XS };
#undef X

// Thanks Simon Tatham.
#define crBegin static int state=0; switch(state) { case 0:
#define crYield do { state=__LINE__; return; case __LINE__:; } while (0)
#define crFinish } (void)0

static void benchmark(enum benchmark_event event, uint64_t elapsed_ms)
{
    static bool is_running = false;
    static int i;
    static int j;
    static uint64_t start;
    static uint64_t totals[2];

    if (!is_running)
    {
        if (BENCHMARK_EVENT_START == event)
        {
            is_running = true;
            start = elapsed_ms;
        }

        return;
    }
    else if (BENCHMARK_EVENT_CANCEL == event)
    {
        is_running = false;
        return;
    }

    assert(BENCHMARK_EVENT_FRAME == event
            || BENCHMARK_EVENT_DONE == event);

    crBegin;
    for (i = 0; i < 2; i++)
    {
        extern bool is_parallel;
        is_parallel = i;
        totals[i] = 0;
        benchmark_reset();

        #define REP 5
        for (j = 0; j < REP; j++)
        {
            crYield;
            while (event != BENCHMARK_EVENT_DONE)
                crYield;

            totals[i] += elapsed_ms - start;
            start = elapsed_ms;

            if (j < REP - 1)
                benchmark_reset();
        }
    }

    SDL_Log("%" PRIu64 " %" PRIu64 "\n", totals[0] / REP, totals[1] / REP);
    crFinish;
}

#define X(L, S) \
void benchmark_##S(uint64_t elapsed_ms) { \
    benchmark(BENCHMARK_EVENT_##L, elapsed_ms); }
BENCHMARK_XS
#undef X

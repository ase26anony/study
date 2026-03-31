/* common.h - Shared header for coverage testing */
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Inline function that will appear in multiple source files */
static inline int common_inline_func(int x) {
    volatile int result = x * 2;
    if (x > 0) {
        result += 1;
    } else {
        result -= 1;
    }
    return result;
}

/* Function prototype for hot function */
int hot_function(int count, int seed);

/* Function prototypes for source files */
void source1_func(int val);
void source2_func(int val);
void source1_unique(void);
void source2_unique(void);

#endif /* COMMON_H */

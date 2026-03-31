#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

/* Inline function that will appear in multiple source files */
static inline int common_inline_func(int x, int y) {
    volatile int result = 0;
    if (x > y) {
        result = x - y;
    } else if (x < y) {
        result = y - x;
    } else {
        result = 1; /* Equal case */
    }
    return result;
}

/* Function prototype for hot function */
int hot_function(int iterations, int seed);

/* Function prototypes for source files */
void source1_func(int seed);
void source2_func(int seed);
int static_func_wrapper(int seed);

#endif /* COMMON_H */

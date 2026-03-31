#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

/* Inline function that will appear in multiple source files */
static inline int common_inline_func(int x) {
    volatile int result = x * 2;
    if (x > 0) {
        result += 10;
    } else {
        result -= 5;
    }
    return result;
}

/* Function prototype for hot function */
int hot_function(int iterations, volatile int* counter);

/* Function prototypes for source files */
void source1_func(int seed, volatile int* checksum);
void source2_func(int seed, volatile int* checksum);

#endif /* COMMON_H */

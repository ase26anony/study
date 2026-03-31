#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

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
int hot_function(int iteration, int seed);

/* Function to generate checksum */
unsigned long generate_checksum(int value);

#endif /* COMMON_H */

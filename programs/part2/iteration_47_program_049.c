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
int hot_function(int iteration, int seed);

/* Function to calculate checksum */
unsigned long calculate_checksum(int value);

#endif /* COMMON_H */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

/* Common inline function that will appear in multiple source files */
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

/* Function to calculate checksum */
unsigned long calculate_checksum(int value);

#endif /* COMMON_H */

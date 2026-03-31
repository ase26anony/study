#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

/* Common inline function that will appear in multiple source files */
static inline int common_inline_func(int x) {
    volatile int result = x * 2 + 1;
    if (x > 0) {
        result += x;
    } else {
        result -= x;
    }
    return result;
}

/* Function prototype for hot function */
int hot_function(int count, int seed);

#endif /* COMMON_H */

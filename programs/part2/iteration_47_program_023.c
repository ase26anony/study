#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

/* Common inline function that will appear in multiple source files */
static inline int common_inline_func(int x) {
    volatile int result = 0;
    if (x > 0) {
        result = x * 2;
    } else {
        result = x - 10;
    }
    
    /* Add some branching for coverage */
    for (int i = 0; i < 3; i++) {
        result += i;
    }
    
    return result;
}

/* Function prototype for hot function */
int hot_function(int count, int seed);

#endif /* COMMON_H */

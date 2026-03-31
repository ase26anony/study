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
int hot_function(int count, int seed);

/* Function prototypes for source files */
void source1_func(int val);
void source2_func(int val);
int static_func_wrapper1(int x);
int static_func_wrapper2(int x);

#endif /* COMMON_H */

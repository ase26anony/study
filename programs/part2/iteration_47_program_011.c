#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

/* Inline function that will appear in multiple source files */
static inline int common_inline_function(int x, int y) {
    volatile int result = 0;
    if (x > y) {
        result = x * 2;
    } else if (x < y) {
        result = y * 3;
    } else {
        result = x + y + 1;
    }
    return result;
}

/* Function prototype for the hot function */
int hot_function(int count, int seed);

/* Function prototypes for source files */
void source1_function(int val);
void source2_function(int val);
int duplicate_static_function_in_source1(int x);
int duplicate_static_function_in_source2(int x);

#endif /* COMMON_H */

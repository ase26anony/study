#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

/* Inline function that will appear in multiple source files */
static inline int common_inline_func(int x) {
    volatile int result = x * 2 + 1;
    if (x > 0) {
        result += x;
    } else {
        result -= x;
    }
    return result;
}

/* Function prototype for the duplicate static function */
int call_static_duplicate(int seed);

#endif /* COMMON_H */

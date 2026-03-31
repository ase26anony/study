#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

/* Inline function that will appear in multiple source files */
static inline int common_inline_func(int x) {
    volatile int result = x * 2;
    if (x > 0) {
        result += 5;
    } else {
        result -= 3;
    }
    return result;
}

/* Function prototype for shared functionality */
int shared_utility(int a, int b);

#endif /* COMMON_H */

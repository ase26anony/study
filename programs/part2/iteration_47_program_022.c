#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

/* Inline function that will appear in multiple source files */
static inline int common_inline_func(int x) {
    volatile int result = x * 2 + 1;
    (void)result; /* Prevent unused variable warning */
    return x * 2;
}

/* Function prototype for hot function */
int hot_function(int iterations);

#endif /* COMMON_H */

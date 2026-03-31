#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

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

/* Function prototype for the duplicate static function */
int call_static_duplicate(void);

#endif /* COMMON_H */

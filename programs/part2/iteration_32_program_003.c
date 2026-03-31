/* hw-doloop-test.c - Test program for hardware loop bitmap intersection analysis */
#include <stdio.h>

volatile int global_counter = 0;

/* Prevent inlining to preserve loop structures */
__attribute__((noinline)) 
int disjoint_loops(int limit) {
    volatile int local = 0;
    
    /* First loop - completely disjoint from second */
    for (int i = 0; i < limit; ++i) {
        local += i * 2;
    }
    
    /* Second loop - no block intersection with first */
    for (int j = 0; j < limit; ++j) {
        local -= j / 2;
    }
    
    return local;
}

__attribute__((noinline))
int perfectly_nested(int outer_limit, int inner_limit) {
    volatile int sum = 0;
    
    /* Outer loop containing inner loop */
    for (int i = 0; i < outer_limit; ++i) {
        /* Inner loop - all blocks are subset of outer loop */
        for (int j = 0; j < inner_limit; ++j) {
            sum += i * j;
        }
        /* Additional outer loop work */
        sum += i;
    }
    
    return sum;
}

__attribute__((noinline))
int partially_overlapping(int limit) {
    volatile int result = 0;
    int temp = 0;
    
    /* First loop with conditional branching */
    for (int i = 0; i < limit; ++i) {
        /* Shared block when condition is true */
        if (i & 1) {
            result += i * 3;
            temp = result;  /* Additional operation in shared block */
        } else {
            /* Unique block for first loop */
            result -= i;
        }
    }
    
    /* Second loop with similar conditional structure */
    for (int j = 0; j < limit; ++j) {
        /* Shared block when condition is true (same condition pattern) */
        if (j & 1) {
            result += j * 2;
            temp = result;  /* Same operation as in first loop's shared block */
        } else {
            /* Unique block for second loop (different from first loop's else) */
            result += j * j;
        }
    }
    
    return result + temp;
}

__attribute__((noinline))
int complex_nesting(int n) {
    volatile int acc = 0;
    
    /* Multiple levels of nesting */
    for (int a = 0; a < n; ++a) {
        for (int b = 0; b < a; ++b) {
            /* Innermost loop - perfectly nested */
            for (int c = 0; c < b; ++c) {
                acc += a * b * c;
            }
        }
        
        /* Another inner loop at different level */
        for (int d = 0; d < n/2; ++d) {
            acc -= a * d;
        }
    }
    
    /* Sequential loop after nested structure */
    for (int e = 0; e < n; ++e) {
        acc += e;
    }
    
    return acc;
}

__attribute__((noinline))
int loops_with_early_exit(int limit) {
    volatile int val = 0;
    
    /* Loop with potential early break */
    for (int i = 0; i < limit; ++i) {
        if (i > limit/2) {
            /* Early exit creates unique control flow */
            break;
        }
        val += i;
    }
    
    /* Another loop with different early exit condition */
    for (int j = 0; j < limit; ++j) {
        if (j % 3 == 0) {
            continue;  /* Skip some iterations */
        }
        val -= j;
        
        if (val < -100) {
            /* Different break condition */
            break;
        }
    }
    
    return val;
}

int main(void) {
    int total = 0;
    
    /* Call all loop patterns to ensure they're analyzed together */
    total += disjoint_loops(10);
    total += perfectly_nested(5, 3);
    total += partially_overlapping(8);
    total += complex_nesting(6);
    total += loops_with_early_exit(12);
    
    /* Update global to prevent dead code elimination */
    global_counter = total;
    
    /* Print result to ensure all code is live */
    printf("Result: %d\n", total);
    
    return total > 0 ? 0 : 1;
}

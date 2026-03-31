/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion pass validation logic
 * Specifically targets lines 577-583 in ifcvt.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent constant propagation */
volatile int global_seed = 42;

/* Function attribute to prevent inlining and preserve CFG */
__attribute__((noinline)) 
int ifcvt_candidate(int init_a, int init_b) {
    int a = init_a;
    int b = init_b;
    
    /* Volatile condition variable - ensures real branch */
    volatile int cond = global_seed;
    
    /* Volatile loop counter to prevent unrolling */
    volatile int iterations = 100;
    
    for (int i = 0; i < iterations; i++) {
        /* The test expression - uses 'cond' but doesn't modify it in then-block */
        if (cond > 0) {
            /* THEN BLOCK: Operations that do NOT modify 'cond' */
            /* These will be checked by the uncovered validation logic */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
            
            /* More operations to ensure block has multiple instructions */
            a = a + (b >> 2);
            b = b - (a % 3);
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify condition variable OUTSIDE the then-block */
        /* This ensures 'cond' is modified, but not between BB_HEAD and then_last_head */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop invariant code motion */
        global_seed = cond % 100;
    }
    
    /* Return value based on computations to prevent dead code elimination */
    return a + b;
}

/* Another test case with different pattern */
__attribute__((noinline))
int ifcvt_candidate2(int x, int y) {
    volatile int threshold = global_seed;
    int result = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Different condition expression */
        if (threshold != 0) {
            /* Safe then-block operations */
            x = y * 3;
            y = x + 7;
            x = x | 0x0F;
            y = y & ~0x03;
            
            /* More arithmetic */
            result += x - y;
            x = x ^ y;
        } else {
            y = x * 2;
            x = y - 5;
        }
        
        /* Modify threshold outside then-block */
        threshold = (threshold + i) % 100;
    }
    
    return result + x + y;
}

int main(void) {
    int result1 = ifcvt_candidate(1, 2);
    int result2 = ifcvt_candidate2(10, 20);
    
    printf("Results: %d, %d\n", result1, result2);
    
    /* Use results to prevent optimization */
    if (result1 > 1000 || result2 > 1000) {
        return 1;
    }
    
    return 0;
}

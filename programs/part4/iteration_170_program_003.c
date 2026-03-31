/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion validation logic
 * for checking that "then" block instructions don't modify the condition variable
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent constant propagation */
volatile int global_seed = 42;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
int ifcvt_candidate(int init_a, int init_b) {
    int a = init_a;
    int b = init_b;
    
    /* Volatile condition variable - ensures real branch generation */
    volatile int cond = global_seed;
    
    /* Volatile loop counter to prevent unrolling */
    volatile int iterations = 100;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition expression using cond (test_expr) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* Simple arithmetic on different variables */
            b = a * 2;      /* Another arithmetic operation */
            a = a ^ b;      /* Bitwise operation */
            b = b & 0xFF;   /* Mask operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond OUTSIDE the conditional blocks */
        /* This ensures cond is loop-variant but not modified in then/else */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop optimization */
        global_seed = i;
    }
    
    /* Return value based on computations to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int nested_if_test(int x, int y) {
    volatile int flag1 = global_seed;
    volatile int flag2 = global_seed + 1;
    int result = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Complex condition that should become test_expr */
        if (flag1 != 0 && flag2 > 0) {
            /* Safe then block - no modification of flag1 or flag2 */
            x = y * 3;
            y = x + 7;
            result += x - y;
            
            /* Inner conditional with safe blocks */
            if (x > y) {
                result += 1;
            }
        }
        
        /* Modify condition variables outside blocks */
        flag1 = (flag1 >> 1) | (flag1 << 31);
        flag2 = flag2 - 1;
    }
    
    return result;
}

/* Test with pointer variables but still safe */
__attribute__((noinline))
int pointer_safe_test(void) {
    int data[4] = {1, 2, 3, 4};
    int temp = 0;
    volatile int selector = global_seed;
    
    for (volatile int i = 0; i < 75; i++) {
        /* Condition based on selector */
        if (selector & 1) {
            /* Safe: modifying array elements, not selector */
            data[0] = data[1] + data[2];
            data[3] = data[0] * 2;
            temp = data[3] - data[0];
        } else {
            data[1] = data[2] - data[3];
            temp = data[1] | 0xAA;
        }
        
        /* Update selector outside conditional */
        selector = (selector * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    return temp + data[0];
}

int main(void) {
    /* Initialize with random-like values */
    int result1 = ifcvt_candidate(1, 2);
    int result2 = nested_if_test(10, 20);
    int result3 = pointer_safe_test();
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    return 0;
}

/* Test program for GCC if-conversion pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function with noinline to prevent inlining and preserve the conditional structure */
__attribute__((noinline))
static int test_if_conversion(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Use volatile to prevent loop unrolling */
    volatile int iterations = 100;
    
    /* Main loop containing the conditional to be if-converted */
    for (volatile int i = 0; i < iterations; i++) {
        /* Read condition from volatile global - this creates test_expr */
        int cond = global_cond;
        
        /* Conditional branch candidate for if-conversion */
        if (cond > 0) {
            /* THEN BLOCK: Operations that do NOT modify 'cond' */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction - masking operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify condition for next iteration (but not in the then block!) */
        /* This ensures the condition changes but isn't modified in the then block */
        global_cond = (global_cond * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Return computed value to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
static int test_if_conversion2(int x, int y) {
    int result = 0;
    volatile int limit = 50;
    
    for (volatile int i = 0; i < limit; i++) {
        /* Complex condition that doesn't get optimized away */
        int cond = (x * y) & 0xF;
        
        /* Conditional with safe then block */
        if (cond != 0) {
            /* Multiple instructions that don't modify cond */
            result = result + x;
            x = x ^ y;
            y = y + 1;
            result = result | y;
        } else {
            result = result - x;
            x = x >> 1;
        }
        
        /* Vary inputs slightly */
        x = (x + 1) & 0xFF;
    }
    
    return result;
}

int main(void) {
    /* Initialize with non-constant values */
    int a = rand() % 100;
    int b = rand() % 100;
    
    /* Call test functions */
    int result1 = test_if_conversion(a, b);
    int result2 = test_if_conversion2(a, b);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}

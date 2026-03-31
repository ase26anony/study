/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
int if_conversion_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile variable for loop control to prevent unrolling */
    volatile int iterations = 100;
    
    /* Main loop where if-conversion will be attempted */
    for (volatile int i = 0; i < iterations; i++) {
        /* Condition variable - read from volatile global */
        int cond = global_cond;
        
        /* 
         * CRITICAL: This is the condition expression (test_expr)
         * that should NOT be modified in the "then" block
         */
        if (cond > 0) {
            /* 
             * THEN BLOCK: Contains instructions that do NOT modify 'cond'
             * These instructions should pass the validation check
             * in ifcvt.cc lines 577-583
             */
            a = b + 1;      /* Simple arithmetic on other variables */
            b = a * 2;      /* Another arithmetic operation */
            a = a ^ b;      /* Bitwise operation */
            b = b & 0xFF;   /* Mask operation */
            
            /* More safe operations that don't touch 'cond' */
            int temp = a + b;
            a = temp - b;
            b = temp * 3;
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
            a = a | 0x0F;
            b = b << 1;
        }
        
        /* 
         * Modify condition variable OUTSIDE the conditional blocks
         * This ensures 'cond' changes across iterations but isn't
         * modified within the then/else blocks
         */
        global_cond = (global_cond * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Return value derived from computations to prevent dead code elimination */
    return a + b;
}

/* Another candidate function with different pattern */
__attribute__((noinline))
int another_candidate(int x, int y) {
    volatile int flag = rand() % 100;
    int result = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        int test_var = flag + i;  /* Condition variable */
        
        /* Conditional with safe then block */
        if (test_var % 2 == 0) {
            /* Safe operations that don't modify test_var */
            x = y * x;
            y = x + i;
            x = x >> 1;
            y = y << 2;
        } else {
            x = y - x;
            y = x * i;
        }
        
        /* Modify test_var source outside conditional */
        flag = (flag * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    return x ^ y;
}

int main() {
    /* Initialize with non-constant values */
    int seed = 42;
    srand(seed);
    
    /* Call if-conversion candidate functions */
    int result1 = if_conversion_candidate(rand(), rand());
    int result2 = another_candidate(rand(), rand());
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}

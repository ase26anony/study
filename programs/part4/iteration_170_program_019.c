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
    
    /* Use volatile local variable for condition to force real branch */
    volatile int cond = global_cond;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int iterations = 100;
    
    for (int i = 0; i < iterations; i++) {
        /* This is the test_expr - uses cond but doesn't modify it in then block */
        if (cond > 0) {
            /* THEN BLOCK: These instructions do NOT modify cond */
            /* They only work on a and b, making them safe for if-conversion */
            a = b + 1;      /* Simple arithmetic - doesn't touch cond */
            b = a * 2;      /* More arithmetic - still safe */
            a = a ^ b;      /* Bitwise operation - safe */
            b = b & 0xFF;   /* Mask operation - safe */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond here (outside both blocks) to change condition across iterations */
        /* This ensures the condition variable changes but not within the then block */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Additional operations to prevent optimization */
        a = a + i;
        b = b - i;
    }
    
    /* Return computed value to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int nested_condition_test(int x, int y) {
    volatile int threshold = global_cond;
    int result = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Multiple conditions to create interesting CFG */
        if (threshold != 0) {
            /* Safe then block - no modification of threshold */
            x = y << 2;
            y = x >> 1;
            result += x | y;
        }
        
        if (x > y) {
            /* Another safe block */
            result = result * 3;
            x = x & 0x0F;
        }
        
        /* Modify condition variable outside blocks */
        threshold = (threshold + i) % 100;
    }
    
    return result;
}

int main(void) {
    /* Initialize with random values to create varying conditions */
    int seed = global_cond;
    srand(seed);
    
    int a = rand() % 1000;
    int b = rand() % 1000;
    
    /* Call the if-conversion candidate function */
    int result1 = if_conversion_candidate(a, b);
    
    /* Call second test function */
    int result2 = nested_condition_test(a, b);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) != 0 ? 0 : 1;
}

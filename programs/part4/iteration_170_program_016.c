/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function with noinline to prevent inlining and preserve the conditional */
__attribute__((noinline)) 
int if_conversion_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile condition variable - ensures real branch generation */
    volatile int cond = global_cond;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression using cond (test_expr) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction - masking operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond for next iteration (loop-variant but safe) */
        /* This ensures the condition changes but isn't in the then block */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop from being optimized away */
        global_cond = cond;
    }
    
    /* Return value derived from a and b to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int another_candidate(unsigned int x, unsigned int y) {
    unsigned int result = 0;
    volatile unsigned int threshold = 500;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Complex condition that doesn't get folded */
        if ((x & 0xF) < (y & 0x7)) {
            /* THEN block with multiple safe instructions */
            unsigned int temp = x >> 2;
            result += temp;
            x = x ^ result;
            y = y | 0x1;
            result = result * 3;
        } else {
            result = result - 1;
            x = x + y;
        }
        
        /* Modify condition variables outside then block */
        x = (x * 1664525 + 1013904223) & 0xFFFFFFFF;
        y = (y * 1103515245 + 12345) & 0xFFFFFFFF;
    }
    
    return result;
}

int main() {
    /* Initialize with random values to create varying conditions */
    int a = rand() % 1000;
    int b = rand() % 1000;
    
    /* Call the if-conversion candidate function */
    int result1 = if_conversion_candidate(a, b);
    
    /* Call another test case */
    unsigned int x = rand() % 1000;
    unsigned int y = rand() % 1000;
    int result2 = another_candidate(x, y);
    
    /* Print results to prevent optimization */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}

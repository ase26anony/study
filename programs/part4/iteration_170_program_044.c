/* ifcvt_coverage.c - Targets GCC's if-conversion validation logic */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent constant propagation */
volatile int global_cond = 1;

/* Function attribute to prevent inlining and early optimization */
__attribute__((noinline))
static int process_data(int iterations) {
    volatile int cond = global_cond;  /* Condition variable - test_expr */
    int a = 0, b = 1;                 /* Working variables */
    int result = 0;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = iterations;
    
    for (int i = 0; i < N; i++) {
        /* Conditional branch with test_expr using 'cond' */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify 'cond' */
            /* These instructions will be validated by the uncovered logic */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
            
            /* More safe operations to ensure block is non-trivial */
            result += (a << 2);
            result ^= (b >> 1);
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
            result -= (a | b);
        }
        
        /* Modify 'cond' outside the conditional blocks */
        /* This ensures it's not modified in the then/else blocks */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop optimization */
        asm volatile("" : "+r" (cond) : : "memory");
    }
    
    return result + a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
static int test_comparison(int x, int y) {
    volatile int compare_var = x - y;  /* test_expr source */
    int accum = 0;
    
    for (int i = 0; i < 50; i++) {
        /* Different condition type */
        if (compare_var != 0) {
            /* Safe then block operations */
            x = y + i;
            y = x * i;
            accum += x & y;
            
            /* More arithmetic that doesn't touch compare_var */
            x = (x << 3) | (y >> 1);
            y = y ^ 0xABCD;
        } else {
            x = y - i;
            y = x / (i + 1);
            accum -= x | y;
        }
        
        /* Modify condition variable AFTER the if block */
        compare_var = (compare_var + i) * 3;
    }
    
    return accum;
}

int main(void) {
    int result1, result2;
    
    /* Initialize with non-deterministic value */
    global_cond = rand() % 100 + 1;
    
    /* First test case */
    result1 = process_data(100);
    
    /* Second test case with different parameters */
    result2 = test_comparison(rand() % 50, rand() % 50);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Also test with edge cases */
    global_cond = 0;
    result1 = process_data(10);
    
    global_cond = 1000;
    result2 = process_data(10);
    
    printf("Edge results: %d, %d\n", result1, result2);
    
    return 0;
}

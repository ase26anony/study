/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent constant propagation */
volatile int global_cond = 1;

/* Function with noinline to prevent inlining and preserve CFG */
__attribute__((noinline))
int if_conversion_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile condition variable - ensures real branch generation */
    volatile int cond = global_cond;
    
    /* Volatile loop limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression using cond (test_expr) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These will be checked by the uncovered validation logic */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
            
            /* More safe operations to ensure block has multiple insns */
            a = a + i;
            b = b - i;
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond for next iteration - but NOT inside then/else blocks */
        /* This ensures cond changes but the validation passes */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop from being optimized away */
        global_cond = cond;
    }
    
    /* Return value derived from a and b to prevent dead code elimination */
    return a + b;
}

/* Second test case with different condition pattern */
__attribute__((noinline))
int another_candidate(int x, int y) {
    volatile int flag = global_cond & 1;
    int result = x;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Different condition expression */
        if (flag == 0) {
            /* Safe then block with various operations */
            result = y << 2;
            y = result | 0x0F;
            result = result + y;
            y = y * 3;
        } else {
            result = y >> 1;
        }
        
        /* Change flag but outside the conditional blocks */
        flag = (flag + i) & 1;
    }
    
    return result;
}

int main() {
    /* Initialize with random values to create variation */
    int seed = global_cond;
    srand(seed);
    
    int a = rand() % 100;
    int b = rand() % 100;
    
    /* Call the if-conversion candidate function */
    int result1 = if_conversion_candidate(a, b);
    
    /* Call another variant */
    int result2 = another_candidate(a, b);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}

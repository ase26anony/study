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
    
    /* Use volatile variable for condition to force real branch */
    volatile int cond = global_cond;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression - test_expr that should not be modified in then block */
        if (cond > 0) {
            /* THEN BLOCK: Operations that do NOT modify 'cond' */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third non-label, non-debug instruction */
            b = b & 0xFF;   /* Fourth non-label, non-debug instruction */
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify condition for next iteration (outside both blocks) */
        /* This ensures cond changes but isn't modified in then/else blocks */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Return value to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int another_candidate(int x, int y) {
    volatile int threshold = 500;
    int result = 0;
    
    /* Different condition expression */
    if (x < threshold) {
        /* Multiple safe operations in then block */
        y = y + x;
        x = x * 2;
        y = y - 1;
        result = x | y;
    } else {
        result = x & y;
    }
    
    return result;
}

int main() {
    /* Initialize with random values to create variation */
    int a = rand() % 1000;
    int b = rand() % 1000;
    
    /* Call the if-conversion candidate function */
    int result1 = if_conversion_candidate(a, b);
    
    /* Call another candidate with different parameters */
    int result2 = another_candidate(a, b);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}

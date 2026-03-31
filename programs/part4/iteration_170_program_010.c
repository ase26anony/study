/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion pass validation logic
 * for checking that instructions in the "then" block don't modify
 * the condition expression.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
int test_if_conversion(int start_a, int start_b) {
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
            /* These should pass the validation in ifcvt.cc lines 577-583 */
            a = b + 1;      /* Simple arithmetic on other variables */
            b = a * 2;      /* Another arithmetic operation */
            a = a ^ b;      /* Bitwise operation - still not modifying cond */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond in a loop-variant way, but outside the then block */
        /* This ensures the condition changes but isn't modified in the then block */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Additional operations to create more RTL instructions */
        b = (a + b) | 0x1;
    }
    
    /* Return value derived from computations to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int test_if_conversion2(int x, int y) {
    volatile int threshold = global_cond;
    int result = 0;
    
    /* Loop with volatile limit */
    volatile int iterations = 50;
    
    for (int i = 0; i < iterations; i++) {
        /* Different condition expression */
        if (threshold != 0) {
            /* THEN BLOCK: Multiple instructions that don't modify threshold */
            int temp = x * y;
            result += temp;
            x = y + i;
            y = x - result;
        } else {
            result -= x * y;
        }
        
        /* Modify threshold outside the then block */
        threshold = (threshold + i) % 100;
    }
    
    return result;
}

/* Main function to drive the tests */
int main() {
    /* Initialize with random values to create variation */
    int seed = global_cond;
    srand(seed);
    
    int a = rand() % 100;
    int b = rand() % 100;
    
    /* Call the test functions */
    int result1 = test_if_conversion(a, b);
    int result2 = test_if_conversion2(a, b);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}

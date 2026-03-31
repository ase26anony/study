/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion pass validation logic
 * for the "then" basic block safety check.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant propagation */
volatile int global_cond = 1;
volatile int global_limit = 100;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
int ifcvt_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    volatile int cond = global_cond;  /* Read from volatile global */
    volatile int N = global_limit;    /* Prevent loop unrolling */
    int i;
    
    /* Loop to increase chances of if-conversion analysis */
    for (i = 0; i < N; i++) {
        /* Condition expression using cond (test_expr) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
            /* Note: cond is NOT modified here */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond for next iteration (loop-variant) */
        /* This ensures the condition changes but is outside the then block */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Additional operations to prevent dead code elimination */
        a = (a + i) & 0xFFF;
        b = (b - i) & 0xFFF;
    }
    
    /* Return value derived from a and b to prevent elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int ifcvt_candidate2(int x, int y) {
    volatile int threshold = global_cond + 50;
    int result = x;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Different condition expression */
        if (x < threshold) {
            /* THEN BLOCK: Multiple arithmetic operations */
            int temp = y * 3;
            result = result + temp;
            y = y ^ result;
            result = result | 0x1;
            /* threshold is NOT modified */
        } else {
            result = y - x;
        }
        
        /* Modify variables for next iteration */
        x = (x + 1) & 0x3FF;
        threshold = (threshold * 3 + 7) & 0xFFF;
    }
    
    return result;
}

int main() {
    int result1, result2;
    
    /* Initialize with random values to prevent constant folding */
    srand(42);
    global_cond = rand() % 100;
    
    /* Call the if-conversion candidate functions */
    result1 = ifcvt_candidate(rand() % 100, rand() % 100);
    result2 = ifcvt_candidate2(rand() % 100, rand() % 100);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}

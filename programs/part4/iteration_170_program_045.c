/* Test program for if-conversion validation logic in ifcvt.cc
 * Specifically targets lines 577-583 checking that instructions
 * in the "then" block don't modify the condition expression
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
int test_if_conversion(int input_a, int input_b) {
    volatile int cond = global_cond;  /* Condition variable - volatile read */
    int a = input_a;
    int b = input_b;
    int result = 0;
    
    /* Volatile loop limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression using 'cond' - this is the test_expr */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify 'cond' */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
            
            /* More safe operations to ensure a substantial block */
            a = a + (b << 2);
            b = b | 0x0F;
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify 'cond' for next iteration (loop-variant but safe) */
        /* This happens AFTER the conditional, so it doesn't affect validation */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Accumulate result to prevent dead code elimination */
        result += a + b;
    }
    
    return result;
}

int main() {
    /* Initialize with random values to create varying conditions */
    int a = rand() % 100;
    int b = rand() % 100;
    
    /* Call the test function multiple times with different inputs */
    int total = 0;
    for (int j = 0; j < 10; j++) {
        total += test_if_conversion(a + j, b - j);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

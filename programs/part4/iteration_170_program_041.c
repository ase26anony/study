/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion validation logic
 * Specifically targets lines 577-583 in ifcvt.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent constant propagation and folding */
volatile int external_cond = 1;

/* Function attribute to prevent inlining and early optimization */
__attribute__((noinline))
static int process_data(int iterations) {
    volatile int cond = external_cond;  /* Condition variable - volatile to prevent const prop */
    int a = 0;                          /* Variable modified in then block */
    int b = 1;                          /* Another variable for operations */
    int c = 2;                          /* Additional variable */
    int d = 3;                          /* Yet another variable */
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = iterations;
    
    for (int i = 0; i < N; i++) {
        /* 
         * CRITICAL: This condition uses 'cond' but the then block 
         * does NOT modify 'cond'. This allows the validation in 
         * noce_find_if_block to succeed.
         */
        if (cond > 0) {
            /* 
             * THEN BLOCK: Contains non-label, non-debug instructions
             * that do NOT modify 'cond'. This is what the uncovered
             * lines are checking for.
             */
            a = b + c;      /* Arithmetic on other variables */
            b = a ^ d;      /* Bitwise operation */
            c = d * 2;      /* More arithmetic */
            d = a & 0xFF;   /* Bitwise AND */
            
            /* Additional safe operations to create more instructions */
            a = b - c;
            c = d | 0x0F;
        } else {
            /* 
             * ELSE BLOCK: Also safe - doesn't modify 'cond'
             * Creates a balanced conditional for if-conversion
             */
            a = b - 1;
            b = c * 3;
            c = d >> 1;
            d = a % 17;
        }
        
        /* 
         * Modify 'cond' here, outside the conditional blocks.
         * This ensures the condition changes across iterations
         * but isn't modified within the then/else blocks.
         */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Additional loop operations to prevent simplification */
        a = a + i;
        b = b - i;
    }
    
    /* Return a value derived from all variables to prevent DCE */
    return a + b + c + d;
}

/* Another test function with different pattern */
__attribute__((noinline))
static int test_complex_condition(int seed) {
    volatile int test_var = seed;
    int x = 0, y = 1, z = 2;
    
    volatile int limit = 50;
    for (int i = 0; i < limit; i++) {
        /* Multiple conditions to create more complex CFG */
        if ((test_var & 1) == 0) {
            /* Safe then block - no modification of test_var */
            x = y + z;
            y = z * x;
            z = x ^ y;
            
            /* More operations to ensure multiple instructions */
            x = x << 1;
            y = y >> 1;
            z = z & 0x0F;
        } else if ((test_var & 2) == 0) {
            /* Another safe branch */
            x = y - z;
            y = z / (x + 1);
            z = x | y;
        } else {
            /* Default case */
            x = y * z;
            y = z + x;
            z = y - x;
        }
        
        /* Modify condition variable outside conditional blocks */
        test_var = (test_var * 1664525 + 1013904223) & 0x7fffffff;
        
        /* Loop-carried dependency */
        z = z + i;
    }
    
    return x + y + z;
}

int main(void) {
    int result1, result2;
    
    /* Initialize with external volatile to prevent constant folding */
    external_cond = rand() % 100 + 1;
    
    /* Call the test functions */
    result1 = process_data(100);
    result2 = test_complex_condition(external_cond);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Additional test with different parameters */
    for (int j = 0; j < 10; j++) {
        external_cond = j;
        int temp = process_data(20);
        printf("Iteration %d: %d\n", j, temp);
    }
    
    return 0;
}

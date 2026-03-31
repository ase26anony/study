/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function attribute to prevent inlining and preserve the conditional structure */
__attribute__((noinline)) 
static int process_conditional(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile variable for loop control to prevent unrolling */
    volatile int iterations = 100;
    
    /* Main loop where if-conversion will be attempted */
    for (volatile int i = 0; i < iterations; i++) {
        /* Condition variable - read from volatile to prevent constant folding */
        int cond = global_cond;
        
        /* 
         * CRITICAL: This is the conditional block that should trigger 
         * the uncovered validation logic in ifcvt.cc
         * The condition uses 'cond' which is NOT modified in the then block
         */
        if (cond > 0) {
            /* 
             * THEN BLOCK: Contains instructions that do NOT modify 'cond'
             * These should pass the validation in lines 577-583 of ifcvt.cc
             */
            a = b + 1;      /* Instruction 1: modifies a, not cond */
            b = a * 2;      /* Instruction 2: modifies b, not cond */
            a = a ^ b;      /* Instruction 3: bitwise operation, not cond */
            b = b & 0xFF;   /* Instruction 4: masking operation, not cond */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* 
         * Modify cond for next iteration - this happens OUTSIDE the conditional
         * This ensures the condition variable changes but isn't modified in the then block
         */
        global_cond = (global_cond * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Return a value derived from a and b to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
static int test_comparison(int x, int y) {
    int result = 0;
    volatile int limit = 50;
    
    for (volatile int i = 0; i < limit; i++) {
        /* Condition based on function arguments */
        int test_var = x + y;
        
        /* Conditional with safe then block */
        if (test_var != 0) {
            /* Multiple instructions that don't modify test_var */
            int temp1 = x * y;
            int temp2 = x ^ y;
            result = temp1 + temp2;
            x = x + 1;      /* Modifies x, not test_var */
            y = y - 1;      /* Modifies y, not test_var */
        } else {
            result = x - y;
        }
        
        /* Modify variables for next iteration */
        x = (x * 3) % 100;
        y = (y * 5) % 100;
    }
    
    return result;
}

int main(void) {
    /* Initialize with random values to create varying conditions */
    int a_val = rand() % 100;
    int b_val = rand() % 100;
    
    /* Call the function to trigger if-conversion */
    int result1 = process_conditional(a_val, b_val);
    int result2 = test_comparison(a_val, b_val);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}

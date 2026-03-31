/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function with noinline attribute to prevent early optimization */
__attribute__((noinline)) 
int if_conversion_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile variable for loop control to prevent unrolling */
    volatile int N = 100;
    
    /* Loop to increase chances of if-conversion analysis */
    for (volatile int i = 0; i < N; i++) {
        /* Condition variable - read from volatile global */
        int test_var = global_cond;
        
        /* Conditional branch with test expression using test_var */
        if (test_var > 0) {
            /* THEN BLOCK: Operations that DO NOT modify test_var */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction - masking operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify test_var */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify test_var for next iteration (but not in then/else blocks) */
        /* This ensures the condition changes but doesn't affect validation */
        global_cond = (global_cond * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Return value derived from a and b to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int if_conversion_candidate2(int x, int y) {
    int result = 0;
    volatile int iterations = 50;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Get condition from volatile source */
        volatile int source = rand() % 100;
        int condition = source;
        
        /* Conditional with test_var in condition */
        if (condition < 50) {
            /* THEN block with multiple safe operations */
            x = y << 1;     /* Shift operation */
            y = x | 0x1;    /* Bitwise OR */
            result += x;    /* Accumulate result */
            y = y + 1;      /* Increment */
        } else {
            /* ELSE block */
            x = y >> 1;
            y = x & 0xFE;
            result -= x;
        }
        
        /* Change condition for next iteration */
        source = (source * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    return result;
}

int main() {
    /* Initialize random seed for variability */
    srand(42);
    
    /* Call the if-conversion candidate functions */
    int result1 = if_conversion_candidate(10, 20);
    int result2 = if_conversion_candidate2(5, 15);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}

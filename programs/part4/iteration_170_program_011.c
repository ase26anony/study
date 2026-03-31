/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent inlining and CFG simplification */
__attribute__((noinline)) 
int if_conversion_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Use volatile variable for condition to force real branch */
    volatile int cond = global_cond;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression - test_expr uses 'cond' */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify 'cond' */
            /* These should pass the validation in lines 577-583 */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction - mask operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify 'cond' in loop-variant way (but outside then/else blocks) */
        /* This ensures condition changes but doesn't affect the validation */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop from being optimized away */
        if (i % 10 == 0) {
            global_cond = cond;
        }
    }
    
    /* Return value derived from a and b to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int another_test(int x, int y) {
    volatile int test_var = x;
    int result = y;
    
    /* Different condition check */
    if (test_var != 0) {
        /* Multiple arithmetic operations that don't modify test_var */
        result = result + (x * y);
        result = result - (x ^ y);
        result = result | 0xABCD;
        result = result << 2;
    } else {
        result = result & 0xFFFF;
    }
    
    /* Slightly modify test_var after the block */
    test_var = test_var + 1;
    
    return result;
}

int main() {
    /* Initialize with random values to create variation */
    int seed = 42;
    srand(seed);
    
    /* Call the if-conversion candidate multiple times */
    int total = 0;
    for (int j = 0; j < 10; j++) {
        int a_val = rand() % 100;
        int b_val = rand() % 100;
        
        total += if_conversion_candidate(a_val, b_val);
        total += another_test(a_val, b_val);
        
        /* Update global condition to affect next iteration */
        global_cond = (global_cond * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}

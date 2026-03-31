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
    
    /* Volatile condition variable - prevents constant folding */
    volatile int cond = global_cond;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression using cond (test_expr) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that do NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction - mask operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond for next iteration (loop-variant but safe) */
        /* This ensures the condition changes but isn't in the then block */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Additional operations to create more RTL instructions */
        a = a + i;
        b = b - i;
    }
    
    /* Return value derived from a and b to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int another_test_case(int x, int y) {
    volatile int test_var = x;
    int result = y;
    
    /* Different condition check */
    if (test_var != 0) {
        /* Multiple safe operations in then block */
        result = result << 2;
        result = result | 0x0F;
        result = result + x;
        result = result - y;
    } else {
        result = result >> 1;
    }
    
    /* Modify test_var outside the then block */
    test_var = test_var * 2;
    
    return result;
}

int main() {
    /* Initialize with random values to create varying conditions */
    int seed = 42;
    srand(seed);
    
    /* Call the if-conversion candidate function */
    int result1 = if_conversion_candidate(rand() % 100, rand() % 100);
    
    /* Call another test case */
    int result2 = another_test_case(rand() % 50, rand() % 50);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}

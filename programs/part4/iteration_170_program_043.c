/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function with noinline attribute to prevent inlining and CFG simplification */
__attribute__((noinline)) 
int if_conversion_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Use volatile variable for condition to force real branch */
    volatile int cond = global_cond;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression (test_expr) using cond */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction - masking operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond in a loop-variant way, but outside the then block */
        /* This ensures cond changes but isn't modified in the then block */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop from being optimized away */
        global_cond = cond;
    }
    
    /* Return value derived from a and b to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int nested_if_test(int x, int y) {
    volatile int cond1 = global_cond;
    volatile int cond2 = global_cond + 1;
    int result = 0;
    
    for (int i = 0; i < 50; i++) {
        /* Compound condition */
        if (cond1 > 0 && cond2 < 100) {
            /* Safe then block - no modification of cond1 or cond2 */
            x = y * 3;
            y = x + 7;
            result += x - y;
        }
        
        /* Update conditions outside then block */
        cond1 = (cond1 * 1664525 + 1013904223) & 0x7fffffff;
        cond2 = (cond2 * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return result + x + y;
}

/* Test with pointer variables but still safe */
__attribute__((noinline))
int pointer_safe_test(void) {
    int data[4] = {1, 2, 3, 4};
    int *ptr1 = &data[0];
    int *ptr2 = &data[2];
    volatile int cond = global_cond;
    int sum = 0;
    
    for (int i = 0; i < 75; i++) {
        if (cond != 0) {
            /* Safe operations that don't modify cond */
            *ptr1 = *ptr2 + 1;
            ptr1++;
            sum += *ptr2;
        } else {
            *ptr2 = *ptr1 - 1;
        }
        
        /* Update cond */
        cond = (cond + i) & 0xFF;
        
        /* Reset pointers periodically */
        if (i % 10 == 0) {
            ptr1 = &data[0];
            ptr2 = &data[2];
        }
    }
    
    return sum;
}

int main(void) {
    /* Initialize random seed for variability */
    srand(42);
    
    /* Test multiple patterns to increase coverage chances */
    int result1 = if_conversion_candidate(rand() % 100, rand() % 100);
    int result2 = nested_if_test(rand() % 100, rand() % 100);
    int result3 = pointer_safe_test();
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    /* Update global for next potential run */
    global_cond = result1 + result2 + result3;
    
    return 0;
}

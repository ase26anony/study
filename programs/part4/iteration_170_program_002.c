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
    
    /* Use volatile to force real conditional */
    volatile int cond = global_cond;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* This is the test_expr - condition based on cond */
        if (cond > 0) {
            /* THEN BLOCK: Operations that do NOT modify cond */
            /* These should pass the validation in lines 577-583 */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third non-label, non-debug instruction */
            b = b & 0xFF;   /* Fourth non-label, non-debug instruction */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond in a loop-variant way (doesn't affect THEN block validation) */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Return value to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int nested_if_test(int x, int y) {
    volatile int cond1 = global_cond;
    volatile int cond2 = global_cond + 1;
    int result = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Compound condition */
        if (cond1 > 0 && cond2 < 100) {
            /* Multiple safe operations in THEN block */
            x = y << 2;
            y = x >> 1;
            result += x | y;
            
            /* Nested safe if */
            if (x > y) {
                result += 1;
            }
        }
        
        /* Update conditions */
        cond1 = (cond1 * 1664525 + 1013904223) & 0x7fffffff;
        cond2 = cond1 % 200;
    }
    
    return result;
}

/* Test with pointer operations (still safe for condition) */
__attribute__((noinline))
int pointer_safe_test(void) {
    int data[4] = {1, 2, 3, 4};
    int *ptr = data;
    volatile int cond = global_cond;
    int sum = 0;
    
    for (volatile int i = 0; i < 30; i++) {
        if (cond & 1) {  /* test_expr using cond */
            /* Safe operations with pointers, doesn't modify cond */
            sum += *ptr;
            ptr++;
            if (ptr > &data[3]) {
                ptr = data;
            }
        } else {
            sum -= 1;
        }
        
        cond = cond ^ (i * 13);
    }
    
    return sum;
}

int main(void) {
    /* Initialize random seed for variability */
    srand(42);
    
    /* Test multiple patterns to increase coverage chance */
    int result1 = if_conversion_candidate(rand() % 100, rand() % 100);
    int result2 = nested_if_test(rand() % 100, rand() % 100);
    int result3 = pointer_safe_test();
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    /* Update global to affect future runs */
    global_cond = rand() % 1000;
    
    return 0;
}

/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
/* Compile with: gcc -O2 -fdump-rtl-ifcvt -fno-tree-loop-if-convert -o ifcvt_test ifcvt_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
int test_if_conversion(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile condition source - prevents constant folding */
    volatile int cond = global_cond;
    
    /* Volatile loop limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression (test_expr) using cond */
        /* This is the condition that should not be modified in the then block */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* Simple arithmetic on other variables */
            b = a * 2;      /* Another arithmetic operation */
            a = a ^ b;      /* Bitwise operation */
            b = b & 0xFF;   /* More bitwise operations */
            
            /* Additional safe operations to create more instructions */
            int temp = a + b;
            a = temp - b;
            b = temp + a;
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond in a loop-variant way, but NOT in the then block */
        /* This ensures the condition changes across iterations */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Additional loop work to prevent other optimizations */
        if (i % 10 == 0) {
            a = a + 1;
        }
    }
    
    /* Return value derived from a and b to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int test_if_conversion2(int x, int y) {
    volatile int threshold = 500;
    int result = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Complex condition that should become test_expr */
        if ((x * y) < threshold) {
            /* THEN block with multiple non-modifying instructions */
            int t1 = x + y;
            int t2 = x - y;
            result = t1 * t2;
            result = result | 0x1;
            x = x ^ result;  /* Modifies x, but not the condition variables */
            y = y & result;  /* Modifies y, but not threshold */
        } else {
            result = x * y;
        }
        
        /* Change threshold but not in the then block */
        threshold = (threshold + i) % 1000;
    }
    
    return result;
}

/* Main function to drive the tests */
int main() {
    /* Initialize random seed for variability */
    srand(42);
    
    /* Test with different inputs to explore different paths */
    int results = 0;
    
    /* First test case */
    results += test_if_conversion(rand() % 100, rand() % 100);
    
    /* Second test case */
    results += test_if_conversion2(rand() % 100, rand() % 100);
    
    /* Third test with edge cases */
    results += test_if_conversion(0, 0);
    results += test_if_conversion(-100, 100);
    
    printf("Result: %d\n", results);
    
    /* Also test with compile-time known but non-constant values */
    int a = 10, b = 20;
    for (volatile int i = 0; i < 10; i++) {
        if (a > b) {
            /* This block should also be validated */
            int tmp = a;
            a = b + 5;
            b = tmp - 3;
        } else {
            a = a + b;
            b = b - a;
        }
        /* Vary the condition */
        a = a + i;
    }
    
    printf("Final values: a=%d, b=%d\n", a, b);
    
    return results != 0 ? 0 : 1;
}

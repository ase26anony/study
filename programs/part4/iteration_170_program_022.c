/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion pass and cover lines 577-583 in ifcvt.cc
 * Compile with: gcc -O2 -fdump-rtl-ifcvt -fno-tree-loop-if-convert -o ifcvt_test ifcvt_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent inlining and CFG simplification */
__attribute__((noinline)) 
int ifcvt_candidate(int start_a, int start_b) {
    volatile int cond = global_cond;  /* Condition variable - volatile read */
    int a = start_a;
    int b = start_b;
    int result = 0;
    
    /* Volatile loop limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression using cond (test_expr) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These instructions should pass the validation in lines 577-583 */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
            
            /* More safe operations to ensure multiple instructions in the block */
            result += a;
            result -= b;
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
            result += b;
        }
        
        /* Modify cond in a loop-variant way that doesn't happen in the then block */
        /* This ensures the condition changes but the then block remains safe */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop optimization */
        asm volatile("" : "+r" (i) : : "memory");
    }
    
    return result + a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int ifcvt_candidate2(int x, int y) {
    volatile int threshold = global_cond + 5;
    int a = x;
    int b = y;
    int c = 0;
    
    volatile int iterations = 50;
    
    for (int i = 0; i < iterations; i++) {
        /* Different condition expression */
        if ((threshold & 1) == 0) {
            /* Safe then block with multiple arithmetic operations */
            a = b << 1;     /* Shift operation */
            b = a | 0x1;    /* Bitwise OR */
            c = a + b;      /* Addition */
            a = c - b;      /* Subtraction */
            b = a * 3;      /* Multiplication */
            
            /* More operations to ensure block is non-trivial */
            c ^= a;
            a |= b;
        } else {
            /* Else block */
            a = b >> 1;
            b = a & 0xFE;
            c = b - a;
        }
        
        /* Modify threshold outside the then block */
        threshold = (threshold + i) & 0xFF;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
    
    return a + b + c;
}

/* Third test case with unsigned types */
__attribute__((noinline))
unsigned int ifcvt_candidate3(unsigned int seed) {
    volatile unsigned int vcond = seed;
    unsigned int x = 0;
    unsigned int y = 1;
    
    volatile unsigned int count = 75;
    
    for (unsigned int i = 0; i < count; i++) {
        /* Condition with unsigned comparison */
        if (vcond < 0x80000000) {
            /* Safe then block with unsigned operations */
            x = y + i;
            y = x * 3;
            x = y / 2;
            y = x % 256;
            x = y ^ 0xAA;
            y = x | 0x55;
            
            /* Additional operation */
            x = x + y;
        } else {
            /* Else block */
            x = y - i;
            y = x / 4;
        }
        
        /* Update condition variable */
        vcond = (vcond * 1664525 + 1013904223) & 0xFFFFFFFF;
        
        /* Prevent optimization */
        asm volatile("" : "+r" (i) : : "memory");
    }
    
    return x + y;
}

int main() {
    /* Initialize random seed for variability */
    srand(42);
    
    /* Update global condition to make it dynamic */
    global_cond = rand();
    
    /* Call all test functions to ensure they're not eliminated */
    int result1 = ifcvt_candidate(rand(), rand());
    int result2 = ifcvt_candidate2(rand(), rand());
    unsigned int result3 = ifcvt_candidate3(rand());
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, %u\n", result1, result2, result3);
    
    /* Also test with different initial conditions */
    global_cond = 0;
    result1 += ifcvt_candidate(1, 2);
    
    global_cond = -1;
    result2 += ifcvt_candidate2(3, 4);
    
    result3 += ifcvt_candidate3(0xFFFFFFFF);
    
    printf("Final results: %d, %d, %u\n", result1, result2, result3);
    
    return (result1 + result2 + result3) != 0 ? 0 : 1;
}

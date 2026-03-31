/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion validation logic for "then" blocks
 * Compile with: gcc -O2 -fdump-rtl-ifcvt ifcvt_coverage.c -o ifcvt_coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline))
int process_data(int iterations) {
    /* Variables for the condition test - use volatile to prevent optimization */
    volatile int test_var = global_cond;
    
    /* Working variables that will be modified in the then/else blocks */
    int a = 0;
    int b = 1;
    int c = 2;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int N = iterations;
    
    for (int i = 0; i < N; i++) {
        /* 
         * CRITICAL: Condition uses test_var, which is NOT modified in the then block
         * This allows the validation in ifcvt.cc lines 577-583 to succeed
         */
        if (test_var > 0) {
            /* 
             * THEN BLOCK: Contains instructions that do NOT modify test_var
             * These are the instructions that will be validated by the uncovered code
             */
            a = b + c;      /* Simple arithmetic - safe */
            b = a ^ 0x55;   /* Bitwise operation - safe */
            c = a * 2;      /* Multiplication - safe */
            
            /* More safe operations to ensure non-empty block */
            a = b & 0xFF;
            c = c | 0x01;
        } else {
            /* ELSE BLOCK: Also doesn't modify test_var */
            a = b - c;
            b = a >> 1;
            c = b * 3;
        }
        
        /* 
         * Modify test_var OUTSIDE the conditional blocks
         * This ensures it changes across iterations but isn't modified in then/else
         */
        test_var = (test_var * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop from being optimized away */
        global_cond = test_var;
    }
    
    /* Return a value based on all variables to prevent dead code elimination */
    return a + b + c;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int test_comparison(int seed) {
    volatile int cond = seed;
    int x = 0, y = 10, z = 20;
    
    volatile int limit = 50;
    for (int i = 0; i < limit; i++) {
        /* Different condition type */
        if (cond != 0) {
            /* Safe operations in then block */
            x = y + z;
            y = x - z;
            z = y * x;
            
            /* More safe operations */
            x = x & ~0x0F;
            y = y | 0x01;
        } else {
            x = z - y;
            y = x ^ 0xAA;
            z = y % 7;
        }
        
        /* Update condition outside the blocks */
        cond = (cond + i) % 100;
    }
    
    return x ^ y ^ z;
}

/* Test with pointer operations (still safe) */
__attribute__((noinline))
int test_with_pointers(int init) {
    volatile int condition = init;
    int data1 = 1, data2 = 2, data3 = 3;
    int *p1 = &data1;
    int *p2 = &data2;
    
    volatile int count = 100;
    for (int i = 0; i < count; i++) {
        if (condition >= 0) {
            /* Safe pointer operations that don't modify condition */
            *p1 = *p2 + data3;
            data2 = *p1 * 2;
            data3 = data1 & data2;
        } else {
            *p2 = data1 - data3;
            data1 = *p2 | 0x0F;
        }
        
        condition = condition * 3 - i;
    }
    
    return data1 + data2 + data3;
}

int main() {
    /* Initialize with random value to prevent constant folding */
    int seed = rand() % 1000;
    
    /* Call test functions with different patterns */
    int result1 = process_data(100);
    int result2 = test_comparison(seed);
    int result3 = test_with_pointers(seed + 1);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    return (result1 + result2 + result3) > 0 ? 0 : 1;
}

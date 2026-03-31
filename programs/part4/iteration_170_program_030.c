/* ifcvt_coverage.c
 * Designed to trigger uncovered lines 577-583 in ifcvt.cc
 * Compile with: gcc -O2 -fdump-rtl-ifcvt -S ifcvt_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
int test_if_conversion(int input_a, int input_b) {
    volatile int cond = global_cond;  /* Condition variable - volatile read */
    int a = input_a;
    int b = input_b;
    int result = 0;
    
    /* Volatile loop counter to prevent unrolling */
    volatile int iterations = 100;
    
    for (int i = 0; i < iterations; i++) {
        /* This is the test expression - uses cond but doesn't modify it in then block */
        if (cond > 0) {
            /* THEN BLOCK: Operations that do NOT modify cond */
            /* These should be safe for if-conversion */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            result += a & b; /* Third instruction - bitwise operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
            result -= a | b;
        }
        
        /* Modify cond for next iteration (but NOT in the then block) */
        /* This ensures the condition changes across iterations */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop optimization */
        asm volatile("" : "+r" (i) : : "memory");
    }
    
    return result + a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int test_bitwise_condition(int x, int y) {
    volatile int mask = global_cond;
    int a = x;
    int b = y;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Different condition expression */
        if ((mask & 0xF) == 0xA) {
            /* Safe then block - no modification of mask */
            a = (a << 1) ^ b;
            b = (b >> 1) ^ a;
            a = a + (b & 0xFF);
        } else {
            a = a ^ b;
            b = b ^ a;
        }
        
        /* Modify mask outside then block */
        mask = (mask << 1) | ((mask >> 31) & 1);
    }
    
    return a ^ b;
}

/* Test with pointer variables but still safe */
__attribute__((noinline))
int test_with_pointers(int init_val) {
    volatile int threshold = global_cond;
    int data[4] = {init_val, init_val + 1, init_val + 2, init_val + 3};
    int sum = 0;
    
    for (volatile int i = 0; i < 40; i++) {
        if (threshold < 1000) {
            /* Safe operations on array elements - doesn't modify threshold */
            data[0] = data[1] + data[2];
            data[1] = data[2] * data[3];
            data[2] = data[0] & data[1];
            sum += data[0] | data[1];
        } else {
            data[0] = data[3] - data[2];
            sum -= data[0];
        }
        
        threshold = (threshold * 3) % 10000;
    }
    
    return sum + data[0];
}

int main() {
    /* Initialize with random values to prevent constant folding */
    int seed = global_cond;
    srand(seed);
    
    int a = rand() % 100;
    int b = rand() % 100;
    
    /* Call test functions */
    int result1 = test_if_conversion(a, b);
    int result2 = test_bitwise_condition(a + 1, b + 1);
    int result3 = test_with_pointers(a + b);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    return (result1 + result2 + result3) != 0 ? 0 : 1;
}

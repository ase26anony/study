/* Program to trigger if-conversion validation logic in GCC's ifcvt.cc */
/* Specifically targets lines 577-583 checking that test_expr isn't modified in then block */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent constant propagation */
volatile int global_seed = 42;

/* Function with noinline to prevent early optimization */
__attribute__((noinline)) 
int if_conversion_candidate(int init_a, int init_b) {
    volatile int cond = global_seed;  /* Condition variable - volatile read */
    int a = init_a;
    int b = init_b;
    int result = 0;
    
    /* Volatile loop counter to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* This is the test_expr - condition based on cond */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            result += a;    /* Third instruction - accumulates result */
            
            /* More safe operations that don't touch cond */
            int temp = a ^ b;
            b = temp & 0xFF;
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
            result -= b;
        }
        
        /* Modify cond here - outside the then/else blocks */
        /* This ensures cond changes across iterations but isn't modified in then block */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Additional loop-invariant operation to prevent over-optimization */
        result = (result + i) & 0xFFFF;
    }
    
    return result + a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int nested_if_test(int x, int y) {
    volatile int threshold = global_seed;
    int acc = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Complex condition using threshold */
        if ((threshold & 1) == 0) {
            /* Safe then block - no modification of threshold */
            x = y << 2;
            y = x >> 1;
            acc += x | y;
            
            /* Another safe operation */
            int mask = 0x0F;
            y = y & mask;
        } else if (threshold > 100) {
            /* Second condition - still safe */
            x = y + threshold;  /* Uses but doesn't modify threshold */
            y = x - threshold;
            acc += x ^ y;
        } else {
            /* Else block */
            x = y * 3;
            y = x % 7;
            acc += x & y;
        }
        
        /* Modify threshold after the conditional blocks */
        threshold = (threshold + i) ^ 0xABCD;
    }
    
    return acc;
}

/* Test with pointer operations but still safe */
__attribute__((noinline))
int pointer_safe_test(void) {
    volatile int flag = global_seed;
    int data[4] = {1, 2, 3, 4};
    int *ptr1 = &data[0];
    int *ptr2 = &data[2];
    int sum = 0;
    
    for (volatile int cnt = 0; cnt < 30; cnt++) {
        if (flag != 0) {
            /* Safe pointer operations that don't modify flag */
            int val1 = *ptr1;
            int val2 = *ptr2;
            *ptr1 = val1 + val2;  /* Modifies data but not flag */
            sum += *ptr1;
            
            /* More safe operations */
            ptr1 = &data[(cnt + 1) % 4];
        } else {
            ptr2 = &data[cnt % 4];
            sum -= *ptr2;
        }
        
        /* Modify flag after the conditional */
        flag = flag * 3 + 1;
    }
    
    return sum;
}

int main(void) {
    /* Initialize with random-like values */
    int result1 = if_conversion_candidate(1, 2);
    int result2 = nested_if_test(5, 7);
    int result3 = pointer_safe_test();
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    /* Also test with different inputs */
    global_seed = 123;
    result1 = if_conversion_candidate(10, 20);
    global_seed = 456;
    result2 = nested_if_test(100, 200);
    
    printf("More results: %d, %d\n", result1, result2);
    
    return 0;
}

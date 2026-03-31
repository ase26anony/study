/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion pass validation logic
 * Specifically targets lines 577-583 in ifcvt.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
int ifcvt_candidate(int init_a, int init_b) {
    int a = init_a;
    int b = init_b;
    
    /* Use volatile to force actual conditional branch */
    volatile int cond = global_cond;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int iterations = 100;
    
    for (int i = 0; i < iterations; i++) {
        /* This is the test_expr - condition using cond */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These should be safe for if-conversion */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction - masking */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond for next iteration (loop-variant but safe) */
        /* This ensures the condition changes but isn't in the then block */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop from being optimized away */
        global_cond = cond;
    }
    
    /* Return value derived from a and b to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int ifcvt_candidate2(unsigned int x, unsigned int y) {
    volatile unsigned int threshold = 1000;
    unsigned int result = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Different condition expression */
        if (x < threshold) {
            /* Safe then block - no modification of x or threshold */
            result = y + (i & 0xF);
            y = result << 1;
            result = result | 0x1;
        } else {
            result = y >> 1;
        }
        
        /* Modify condition variables outside the then block */
        x = (x * 3 + 1) & 0xFFF;
        threshold = (threshold + 17) & 0xFFF;
    }
    
    return result;
}

/* Test with pointer arithmetic but still safe */
__attribute__((noinline))
int ifcvt_candidate3(int *ptr1, int *ptr2) {
    int temp1 = 0, temp2 = 0;
    volatile int flag = *ptr1;
    
    for (int i = 0; i < 30; i++) {
        if (flag != 0) {
            /* Safe operations - modifying different variables */
            temp1 = temp2 + *ptr2;
            temp2 = temp1 * i;
            temp1 = temp1 & 0xFFFF;
        } else {
            temp2 = temp1 - *ptr2;
        }
        
        /* Modify flag outside then block */
        flag = (flag + i) & 0x7F;
        *ptr2 = *ptr2 + 1;  /* External side effect */
    }
    
    return temp1 + temp2;
}

int main() {
    /* Initialize with random values to prevent constant folding */
    int a = rand() % 100;
    int b = rand() % 100;
    
    /* Call the if-conversion candidate functions */
    int result1 = ifcvt_candidate(a, b);
    int result2 = ifcvt_candidate2(a, b);
    
    int arr1 = 42, arr2 = 99;
    int result3 = ifcvt_candidate3(&arr1, &arr2);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    return 0;
}

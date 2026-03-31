/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion pass validation logic
 * for checking that instructions in the "then" block don't modify
 * the condition expression.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline))
static int process_data(int iterations) {
    volatile int cond = global_cond;  /* Condition variable - volatile read */
    int a = 0;                        /* Variable a - used in then block */
    int b = 1;                        /* Variable b - used in then block */
    int result = 0;
    
    /* Volatile loop limit to prevent unrolling */
    volatile int N = iterations;
    
    for (int i = 0; i < N; i++) {
        /* 
         * This is the critical conditional.
         * The condition expression uses 'cond' which is NOT modified
         * in the then block, passing the validation check.
         */
        if (cond > 0) {
            /* THEN BLOCK: Operations that do NOT modify 'cond' */
            /* These instructions should pass the modified_in_p check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction - masking operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* 
         * Update 'cond' here, outside the conditional blocks.
         * This ensures the condition changes across iterations
         * but isn't modified within the then/else blocks.
         */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Accumulate result to prevent dead code elimination */
        result += a + b;
    }
    
    return result;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
static int test_comparison(int seed) {
    volatile int x = seed;
    int y = 0, z = 0;
    
    volatile int limit = 50;
    for (int i = 0; i < limit; i++) {
        /* Condition using multiple comparisons */
        if (x != 0 && x < 100) {
            /* Safe then block - no modification of x */
            y = z + i;
            z = y * i;
            y = y | 0x1;     /* Bitwise OR */
            z = z << 2;      /* Shift operation */
        } else {
            /* Else block */
            y = z - i;
            z = y >> 1;
        }
        
        /* Modify condition variable outside blocks */
        x = (x + i) % 200;
    }
    
    return y + z;
}

/* Test with pointer operations but still safe */
__attribute__((noinline))
static int test_with_pointers(int init) {
    volatile int flag = init;
    int data1 = 10, data2 = 20;
    int *p1 = &data1;
    int *p2 = &data2;
    
    volatile int count = 30;
    for (int i = 0; i < count; i++) {
        if (flag % 2 == 0) {
            /* Operations through pointers, but still not modifying flag */
            *p1 = *p2 + i;
            *p2 = *p1 - i;
            data1 = data1 & 0xF;  /* Bitwise AND */
            data2 = data2 | 0x1;  /* Bitwise OR */
        } else {
            *p1 = *p2 - i;
            *p2 = *p1 + i;
        }
        
        flag = (flag * 3 + 1) % 100;
    }
    
    return data1 + data2;
}

int main() {
    int result1, result2, result3;
    
    /* Initialize with non-deterministic value if possible */
    global_cond = rand() % 100 + 1;
    
    printf("Testing if-conversion coverage...\n");
    
    /* Call test functions with different patterns */
    result1 = process_data(100);
    result2 = test_comparison(42);
    result3 = test_with_pointers(7);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    return (result1 + result2 + result3) > 0 ? 0 : 1;
}

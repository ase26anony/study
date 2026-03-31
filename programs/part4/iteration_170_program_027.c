/* ifcvt_coverage.c
 * Designed to trigger specific validation logic in GCC's if-conversion pass
 * Compile with: gcc -O2 -fdump-rtl-ifcvt ifcvt_coverage.c -o ifcvt_coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent inlining and CFG simplification */
__attribute__((noinline)) 
int process_data(int iterations) {
    /* Local volatile to ensure real conditional branch */
    volatile int cond = global_cond;
    
    /* Variables to be modified in the then/else blocks */
    int a = 0;
    int b = 1;
    int result = 0;
    
    /* Volatile loop limit to prevent unrolling */
    volatile int N = iterations;
    
    for (int i = 0; i < N; i++) {
        /* 
         * CRITICAL: Condition uses 'cond' but the then block 
         * must NOT modify 'cond' to pass the validation check
         */
        if (cond > 0) {
            /* 
             * THEN BLOCK: Safe operations that do NOT modify 'cond'
             * These generate non-label, non-debug instructions
             */
            a = b + 1;      /* Arithmetic operation */
            b = a * 2;      /* Another arithmetic operation */
            result ^= a;    /* Bitwise operation */
        } else {
            /* 
             * ELSE BLOCK: Also safe, doesn't modify 'cond'
             */
            a = b - 1;      /* Different arithmetic */
            b = a / 2;      /* Division operation */
            result |= b;    /* Bitwise operation */
        }
        
        /* 
         * Modify 'cond' OUTSIDE the conditional blocks
         * This ensures the condition variable changes across iterations
         * but is not modified within the then/else blocks
         */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Additional loop operation to prevent optimization */
        result += i;
    }
    
    return result;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int test_complex_condition(int seed) {
    volatile int cond1 = seed;
    volatile int cond2 = seed + 1;
    
    int x = 0, y = 0, z = 0;
    volatile int limit = 50;
    
    for (int i = 0; i < limit; i++) {
        /* Compound condition using cond1 and cond2 */
        if ((cond1 & 1) && (cond2 > 0)) {
            /* Safe then block - no modification of cond1 or cond2 */
            x = y + z;
            y = x ^ i;
            z = y << 2;
        } else if (cond1 < 0) {
            /* Another safe block */
            x = y - z;
            y = x | i;
            z = y >> 1;
        } else {
            /* Default safe block */
            x = y * z;
            y = x & i;
            z = y + 1;
        }
        
        /* Modify condition variables outside blocks */
        cond1 = (cond1 >> 1) | (cond1 << 31);
        cond2 = cond2 * 1664525 + 1013904223;
    }
    
    return x + y + z;
}

int main() {
    /* Initialize with random value to vary condition */
    global_cond = rand() % 100;
    
    printf("Starting if-conversion coverage test...\n");
    
    /* Call the function with different iteration counts */
    int result1 = process_data(100);
    int result2 = test_complex_condition(global_cond);
    
    printf("Results: %d, %d\n", result1, result2);
    
    /* Use results to prevent dead code elimination */
    if (result1 > 0 && result2 > 0) {
        printf("Test completed successfully\n");
    }
    
    return 0;
}

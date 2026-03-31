/* Program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i, j;
    
    /* Outer loop to provide multiple scheduling contexts */
    for (j = 0; j < 10; j++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations:
               - Multiplication creates multi-cycle latency
               - Addition provides another operation
               - Shift breaks potential pattern recognition
               This creates a good candidate for instruction movement */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            a[i] = a[i] + (sum & 0x1);  /* Simple modification */
        }
        
        /* Modify input slightly for outer loop variation */
        b[0] += sum;
        b[31] ^= sum;
    }
    
    return sum;
}

/* Another test function with different pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test2(int *arr, int n) {
    int acc1 = 7, acc2 = 3;
    int i, j;
    
    for (j = 0; j < 5; j++) {
        /* Loop with two interleaved recurrences */
        for (i = 1; i < n; i++) {
            /* True data dependency: uses previous iteration's result */
            acc1 = (acc1 * 3 + arr[i]) % 1001;
            acc2 = (acc2 + acc1 * arr[i-1]) & 0xFF;
            
            /* Store back to create memory dependencies */
            arr[i] = acc1 + acc2;
        }
        
        /* Cross-iteration dependency */
        arr[0] = acc1 + acc2;
    }
    
    return acc1 + acc2;
}

int main(void) {
    int a[128], b[128];
    int arr[64];
    int i, result1, result2;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values
       Using volatile-like behavior to prevent optimization */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    for (i = 0; i < 64; i++) {
        arr[i] = rand() % 256;
    }
    
    /* Call test functions */
    result1 = modulo_sched_test(a, b, 32);
    result2 = modulo_sched_test2(arr, 32);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}

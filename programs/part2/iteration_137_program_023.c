/* Program to trigger modulo scheduling debug logging in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test(int *a, int *b, int n) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i, j;
    
    /* Outer loop to provide multiple scheduling contexts */
    for (j = 0; j < 10; j++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations:
               - Multiplication with carried dependency (sum * a[i])
               - Addition with array element
               - Shift operation
               This creates a challenging scheduling problem */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional arithmetic to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            sum = sum + (b[i] % 16);
        }
        
        /* Modify input slightly to create loop-variant behavior for outer loop */
        b[0] += sum;
        a[1] ^= sum;
    }
    
    return sum;
}

/* Another test function with different recurrence pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test2(int *arr, int n) {
    int acc1 = 5, acc2 = 7;
    int i, j;
    
    for (j = 0; j < 8; j++) {
        /* Loop with multiple interleaved recurrences */
        for (i = 1; i < 64; i++) {
            /* Two separate carried dependencies */
            acc1 = (acc1 * 3 + arr[i]) % 1000;
            acc2 = (acc2 + acc1 * arr[i-1]) & 0x3FF;
            
            /* Cross-dependency between the two accumulators */
            arr[i] = (arr[i] + acc1 - acc2) >> 2;
        }
        
        /* Small modification to prevent complete optimization */
        arr[0] = acc1 + acc2;
    }
    
    return acc1 + acc2;
}

int main() {
    int a[128], b[128];
    int arr[128];
    int i, result1, result2;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values
       Using volatile-like behavior to prevent constant propagation */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        arr[i] = rand() % 256;
    }
    
    /* Call test functions */
    result1 = modulo_sched_test(a, b, 32);
    result2 = modulo_sched_test2(arr, 64);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Additional volatile store to ensure computations aren't optimized away */
    volatile int dummy = result1 + result2;
    
    return 0;
}

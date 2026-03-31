/* Program to trigger modulo scheduling debug output in GCC */
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
        /* Fixed small iteration count for manageable scheduling problem */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations:
               - Multiplication with carried dependency (sum * a[i])
               - Addition with array element
               - Shift operation
               This creates a good mix of operations for scheduling */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional arithmetic to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            
            /* Another operation using the result */
            if (sum < 0) sum = -sum;
        }
        
        /* Modify input slightly to create loop-variant behavior for outer loop */
        b[0] += sum % 100;
    }
    
    return sum;
}

/* Another test function with different recurrence pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test2(int *arr, int n) {
    int acc1 = 1, acc2 = 2;
    int i, j;
    
    for (j = 0; j < 5; j++) {
        /* Loop with multiple interleaved recurrences */
        for (i = 1; i < 64; i++) {
            /* Two separate carried dependencies */
            acc1 = (acc1 * 3 + arr[i]) % 1000;
            acc2 = (acc2 + acc1 * arr[i-1]) >> 1;
            
            /* Cross-dependency between the two accumulators */
            arr[i] = (acc1 + acc2) & 0xFF;
        }
        
        /* Use volatile to prevent optimization */
        volatile int dummy = acc1;
        arr[0] = dummy;
    }
    
    return acc1 + acc2;
}

int main(void) {
    int a[128], b[128];
    int i, result1, result2;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values
       to prevent constant propagation */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    /* Use volatile to ensure values aren't optimized away */
    volatile int *volatile_a = a;
    volatile int *volatile_b = b;
    
    printf("Starting modulo scheduling test...\n");
    
    /* Call test functions - compiler won't know they have no side effects */
    result1 = modulo_sched_test((int*)volatile_a, (int*)volatile_b, 128);
    result2 = modulo_sched_test2((int*)volatile_a, 128);
    
    printf("Results: %d, %d\n", result1, result2);
    
    /* Use results to prevent dead code elimination */
    return (result1 + result2) > 0 ? 0 : 1;
}

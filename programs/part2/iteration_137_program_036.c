/* Program to trigger GCC modulo scheduler debug logging */
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
               - Multiplication with carried dependency (sum * a[i])
               - Addition with array element
               - Shift operation
               This creates a challenging scheduling problem */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional arithmetic to increase instruction count */
            sum ^= (a[i] & 0xFF);  /* XOR operation */
            sum += (i & 1);        /* Conditional-like addition */
        }
        
        /* Modify input slightly to prevent complete optimization */
        b[0] += sum & 1;
        a[31] ^= j;
    }
    
    return sum;
}

/* Another test function with different recurrence pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test2(int *arr, int n) {
    int acc1 = 3, acc2 = 5;
    int i, j;
    
    for (j = 0; j < 8; j++) {
        /* Double recurrence with interleaved dependencies */
        for (i = 1; i < n && i < 64; i++) {
            /* Two separate carried dependencies */
            acc1 = (acc1 * 3 + arr[i]) % 1001;
            acc2 = (acc2 + acc1 * arr[i-1]) >> 2;
            
            /* More operations to increase scheduling complexity */
            arr[i] = (arr[i] ^ acc2) + i;
        }
        
        /* Prevent optimization */
        arr[0] = acc1;
    }
    
    return acc1 + acc2;
}

int main(void) {
    int a[128], b[128];
    int i, result1, result2;
    
    /* Initialize with pseudo-random values to prevent constant propagation */
    srand(time(NULL));
    
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Use volatile to prevent optimization */
    volatile int size = 64;
    
    /* Call test functions */
    result1 = modulo_sched_test(a, b, size);
    
    /* Re-initialize for second test */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 100;
    }
    
    result2 = modulo_sched_test2(a, 64);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}

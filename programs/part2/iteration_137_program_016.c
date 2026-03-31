/* Program to trigger GCC modulo scheduling debug output */
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
        /* Inner loop with carried dependency for modulo scheduling */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations:
               - Multiplication with carried dependency (sum * a[i])
               - Addition with array element
               - Shift operation
               This creates a challenging scheduling problem */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum ^= (a[i] & 0xFF);      /* Bitwise operation */
            sum += (i & 0x3);          /* Small constant addition */
        }
        
        /* Modify input slightly to prevent complete optimization */
        b[0] += sum & 0x1;
        a[j % 32] ^= sum;
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
            acc2 = (acc2 + acc1 * arr[i-1]) >> 2;
            
            /* Cross dependency between accumulators */
            arr[i] = (arr[i] + acc1 + acc2) & 0xFF;
        }
        
        /* Prevent optimization */
        arr[0] = (arr[0] + acc1) & 0xFF;
    }
    
    return acc1 + acc2;
}

int main() {
    int a[128], b[128];
    int i, result1, result2;
    
    /* Initialize with pseudo-random values to prevent constant propagation */
    srand(time(NULL));
    
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    /* Call the test functions */
    result1 = modulo_sched_test(a, b, 32);
    printf("Result 1: %d\n", result1);
    
    /* Reinitialize for second test */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
    }
    
    result2 = modulo_sched_test2(a, 64);
    printf("Result 2: %d\n", result2);
    
    return 0;
}

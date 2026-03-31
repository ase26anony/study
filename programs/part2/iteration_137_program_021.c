/* Program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops", "O2")))
int modulo_sched_test(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i, j;
    
    /* Outer loop to provide multiple scheduling contexts */
    for (j = 0; j < 10; j++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        /* Fixed small iteration count for manageable scheduling problem */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations */
            /* sum depends on previous iteration's sum */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            sum = sum + (b[i] % 16);
        }
        
        /* Modify input slightly to prevent complete optimization */
        b[0] += sum & 1;
        a[1] ^= sum & 0xFF;
    }
    
    return sum;
}

/* Another test function with different pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops", "O2")))
int modulo_sched_test2(int *arr, int n) {
    int acc1 = 1, acc2 = 2;
    int i, j;
    
    for (j = 0; j < 5; j++) {
        /* Loop with multiple interleaved dependencies */
        for (i = 1; i < 64; i++) {
            /* Cross-iteration dependencies */
            acc1 = arr[i] * acc1 + arr[i-1];
            acc2 = acc2 + acc1 * 3;
            acc1 = acc1 ^ (acc2 >> 2);
        }
        
        /* Prevent optimization */
        arr[0] = acc1;
        arr[1] = acc2;
    }
    
    return acc1 + acc2;
}

int main() {
    int a[128], b[128];
    int i, result1, result2;
    
    srand(time(NULL));
    
    /* Initialize with volatile-like behavior using rand() */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    /* Call test functions */
    result1 = modulo_sched_test(a, b, 32);
    result2 = modulo_sched_test2(a, 64);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}

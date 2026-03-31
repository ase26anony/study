/* Program to trigger modulo scheduling debug logging in GCC */
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
        /* Fixed small iteration count for manageable scheduling problem */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations:
               - Multiplication with carried dependency (sum * a[i])
               - Addition with array element
               - Shift operation
               This creates multiple instructions for the scheduler */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional arithmetic to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);  /* XOR operation */
            
            /* Another carried dependency */
            sum = sum + (sum >> 3);
        }
        
        /* Modify input slightly to prevent complete optimization */
        b[0] += sum & 1;
        a[1] ^= sum & 0xFF;
    }
    
    return sum;
}

/* Another test function with different pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test2(int *arr, int n) {
    int acc1 = 1, acc2 = 2;
    int i, j;
    
    for (j = 0; j < 5; j++) {
        /* Different carried dependency pattern */
        for (i = 1; i < 64; i++) {
            /* Multiple interleaved recurrences */
            acc1 = arr[i] * acc1 + acc2;
            acc2 = arr[i-1] + acc1;
            
            /* More operations to create scheduling opportunities */
            int temp = acc1 - acc2;
            acc1 = temp >> 2;
            acc2 = (acc2 * 3) & 0x7FFF;
        }
        
        /* Prevent optimization */
        arr[0] = acc1;
    }
    
    return acc1 + acc2;
}

int main() {
    int a[128], b[128];
    int i;
    
    /* Initialize with pseudo-random values to prevent constant propagation */
    srand(time(NULL));
    
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile int result1 = modulo_sched_test(a, b, 32);
    volatile int result2 = modulo_sched_test2(a, 64);
    
    /* Print results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}

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
        /* Inner loop with carried dependency for modulo scheduling */
        /* Fixed small iteration count (32) for manageable scheduling */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations:
               - Multiplication with carried dependency (sum * a[i])
               - Addition with array element
               - Shift operation
               This creates multiple instructions for the scheduler */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional arithmetic to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);  /* XOR operation */
            sum = sum + (i & 0x3);      /* Small addition */
        }
        
        /* Modify input slightly to prevent complete optimization */
        b[0] += sum & 0x1;
        a[31] ^= sum & 0x3;
    }
    
    return sum;
}

/* Another test function with different recurrence pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test2(int *arr, int n) {
    int acc1 = 5, acc2 = 7;
    int i, j;
    
    for (j = 0; j < 8; j++) {
        /* Double recurrence with cross dependencies */
        for (i = 1; i < 64; i++) {
            /* Two interleaved recurrences */
            acc1 = (acc1 * 3 + arr[i]) % 1001;
            acc2 = (acc2 + acc1 * arr[i-1]) & 0xFFF;
            
            /* Additional operations for more scheduling candidates */
            int temp = acc1 - acc2;
            acc1 = acc1 ^ (temp >> 2);
            acc2 = acc2 | (temp & 0x3F);
        }
        
        /* Prevent optimization */
        arr[0] = (arr[0] + acc1) & 0xFF;
    }
    
    return acc1 + acc2;
}

int main(void) {
    int a[128], b[128];
    int arr[128];
    int i, result1, result2;
    
    /* Seed random number generator for unpredictable values */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values
       to prevent constant propagation */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        arr[i] = rand() % 1000;
    }
    
    /* Use volatile to ensure values are actually read */
    volatile int *volatile_a = a;
    volatile int *volatile_b = b;
    volatile int *volatile_arr = arr;
    
    /* Call test functions multiple times */
    result1 = modulo_sched_test((int*)volatile_a, (int*)volatile_b, 128);
    result2 = modulo_sched_test2((int*)volatile_arr, 128);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}

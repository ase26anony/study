/* Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fno-tree-vectorize -fno-unroll-loops -o modulo_test modulo_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function attribute to prevent unwanted optimizations */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop(int* restrict a, int* restrict b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    
    /* Outer loop to provide multiple scheduling contexts */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        for (int i = 0; i < size; i++) {
            /* Complex recurrence with multiple operations:
               - Multiplication with carried dependency (sum * a[i])
               - Addition with array element
               - Shift operation
               This creates multiple instructions for the scheduler */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional arithmetic to increase instruction count */
            sum ^= (sum << 3) | (sum >> 29);  /* Simple mixing */
        }
        
        /* Modify input slightly to prevent complete optimization */
        if (outer % 3 == 0) {
            b[0] += sum;
        }
    }
    
    return sum;
}

/* Another variant with different dependency pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop2(int* restrict arr, int size) {
    int acc1 = 5, acc2 = 7;
    
    for (int outer = 0; outer < 8; outer++) {
        /* Loop with two interleaved carried dependencies */
        for (int i = 0; i < size; i++) {
            /* Two separate recurrence chains */
            acc1 = (acc1 * 3 + arr[i]) & 0xFF;
            acc2 = (acc2 + acc1 * arr[size - i - 1]) >> 1;
            
            /* Cross dependency between the two chains */
            arr[i] = (acc1 ^ acc2) + i;
        }
    }
    
    return acc1 + acc2;
}

int main(void) {
    const int SIZE = 64;  /* Small, compile-time constant size */
    int a[SIZE], b[SIZE];
    int arr[SIZE];
    
    /* Initialize with volatile-like behavior using rand()
       to prevent constant propagation */
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (rand() % 256) + 1;    /* Avoid zero for multiplication */
        b[i] = rand() % 256;
        arr[i] = rand() % 256;
    }
    
    /* Volatile variable to prevent dead code elimination */
    volatile int result1, result2;
    
    /* Call the computation functions */
    result1 = compute_loop(a, b, SIZE);
    result2 = compute_loop2(arr, SIZE);
    
    /* Print results to prevent optimization */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    
    return 0;
}

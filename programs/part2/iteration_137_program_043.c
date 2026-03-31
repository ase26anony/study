/* Program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i;
    
    /* Inner loop with carried dependency - critical for modulo scheduling */
    for (i = 0; i < size; ++i) {
        /* Complex recurrence with multiple operations */
        sum = (sum * a[i] + b[i]) >> 1;
        
        /* Additional operations to create more scheduling opportunities */
        sum = sum ^ (a[i] & 0xFF);
        sum = sum + (b[i] % 256);
        
        /* Another carried dependency */
        a[i] = (a[i] + sum) & 0xFFFF;
    }
    
    return sum;
}

/* Another function with different pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop2(int *a, int *b, int size) {
    int sum = a[0];
    int i;
    
    /* Different recurrence pattern */
    for (i = 1; i < size; ++i) {
        /* Multiple interleaved dependencies */
        int temp = sum * 3;
        sum = temp + a[i] * b[i];
        b[i-1] = sum >> 2;
        sum = sum ^ (temp & 0xFF);
    }
    
    return sum;
}

int main(void) {
    const int ARRAY_SIZE = 64;  /* Small, compile-time constant */
    int a[ARRAY_SIZE], b[ARRAY_SIZE];
    int i, j;
    int total_sum = 0;
    
    /* Initialize with pseudo-random values to prevent constant propagation */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Outer loop to increase scheduling analysis opportunities */
    for (j = 0; j < 10; ++j) {
        /* Call the critical inner loop function */
        int result = compute_loop(a, b, 32);  /* Fixed small size */
        total_sum += result;
        
        /* Modify arrays slightly to create loop-variant behavior */
        a[0] += result % 100;
        b[0] += j;
        
        /* Call second pattern */
        result = compute_loop2(a, b, 32);
        total_sum += result;
        
        /* Shuffle array values slightly */
        for (i = 1; i < ARRAY_SIZE; ++i) {
            a[i] = (a[i] + a[i-1]) & 0x7FF;
        }
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = total_sum;
    printf("Result: %d\n", final_result);
    
    return 0;
}

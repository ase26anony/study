/* Program to trigger modulo scheduling debug logging in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling to focus on modulo scheduling */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop(int* a, int* b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    
    /* Outer loop to provide multiple scheduling contexts */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        for (int i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum ^= (a[i] & 0xFF);
            sum += (b[i] % 16);
        }
        
        /* Modify input slightly to prevent complete optimization */
        b[0] += sum & 1;
        a[outer % 32] ^= sum;
    }
    
    return sum;
}

/* Another function with different recurrence pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop2(int* a, int* b, int size) {
    int acc = a[0];
    
    /* Different recurrence: a[i] depends on a[i-1] */
    for (int i = 1; i < 64; i++) {
        acc = (acc * 3 + b[i]) / 2;
        a[i] = acc + a[i-1];
    }
    
    return acc;
}

/* Function with multiple accumulators */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop3(int* a, int* b, int size) {
    int sum1 = 1, sum2 = 2;
    
    for (int i = 0; i < 48; i++) {
        /* Interleaved dependencies */
        sum1 = sum1 * a[i] + b[i];
        sum2 = sum2 + sum1 * 7;
        sum1 = sum1 ^ sum2;
    }
    
    return sum1 + sum2;
}

int main() {
    /* Initialize with volatile-like behavior to prevent constant propagation */
    srand(time(NULL));
    
    int a[128], b[128];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 128; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call multiple loop functions to increase scheduling opportunities */
    int result1 = compute_loop(a, b, 32);
    int result2 = compute_loop2(a, b, 64);
    int result3 = compute_loop3(a, b, 48);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    return 0;
}

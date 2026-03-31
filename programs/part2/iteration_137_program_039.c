/* Program to trigger GCC modulo scheduling debug output */
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
        /* Additional operations to increase instruction count */
        sum = sum ^ (a[i] & 0xFF);
        sum = sum + (b[i] % 256);
    }
    
    return sum;
}

/* Another loop variant with different dependency pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop2(int *a, int *b, int size) {
    int sum = a[0];
    int i;
    
    /* Different carried dependency pattern */
    for (i = 1; i < size; ++i) {
        /* sum depends on previous iteration's sum */
        sum = sum * 3 + a[i];
        /* Cross-iteration dependency through array */
        b[i] = b[i-1] + sum;
        /* More arithmetic operations */
        sum = (sum + b[i]) & 0xFFFF;
    }
    
    return sum;
}

int main(void) {
    const int ARRAY_SIZE = 64;
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    int i, j;
    int total_sum = 0;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Outer loop to provide multiple scheduling contexts */
    for (j = 0; j < 20; ++j) {
        /* Call the inner loop with carried dependency */
        total_sum += compute_loop(a, b, 32);  /* Fixed small size */
        
        /* Alternate with different loop pattern */
        if (j % 3 == 0) {
            total_sum += compute_loop2(a, b, 32);
        }
        
        /* Modify input arrays slightly to create outer loop variation */
        a[0] += total_sum & 0xF;
        b[0] += j;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d\n", total_sum);
    
    return 0;
}

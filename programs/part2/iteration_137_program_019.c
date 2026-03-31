/* Program to trigger modulo scheduling debug logging in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling to focus on modulo scheduling */
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
        
        /* Another recurrence to create more dependencies */
        if (i > 0) {
            sum = sum + (a[i-1] * 3);
        }
    }
    
    return sum;
}

/* Outer loop to provide multiple scheduling contexts */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int outer_loop(int *a, int *b, int inner_size, int outer_iters) {
    int total = 0;
    int j;
    
    for (j = 0; j < outer_iters; ++j) {
        /* Call inner computation */
        int result = compute_loop(a, b, inner_size);
        total += result;
        
        /* Modify input arrays slightly to prevent optimization */
        a[0] += j;
        b[j % inner_size] ^= result & 0xFF;
    }
    
    return total;
}

int main() {
    const int ARRAY_SIZE = 64;
    const int OUTER_ITERS = 10;
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    int i;
    
    /* Initialize with pseudo-random values to prevent constant propagation */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile int seed = rand();
    a[0] ^= seed;
    b[0] ^= seed;
    
    /* Execute the computation */
    int result = outer_loop(a, b, 32, OUTER_ITERS);
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", result);
    
    /* Additional test with different pattern */
    int c[ARRAY_SIZE];
    int d[ARRAY_SIZE];
    
    for (i = 0; i < ARRAY_SIZE; ++i) {
        c[i] = i * 3;
        d[i] = i * 7;
    }
    
    /* Another loop with different dependency pattern */
    int sum2 = 1;
    for (i = 1; i < 48; ++i) {
        sum2 = (sum2 * c[i] + d[i-1]) / 2;
        sum2 = sum2 | (c[i] << 3);
    }
    
    printf("Sum2: %d\n", sum2);
    
    return 0;
}

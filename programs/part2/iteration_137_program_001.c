/* Program to trigger GCC modulo scheduling debug output in modulo-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling to focus on modulo scheduling */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
static int compute_loop(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i;
    
    /* Inner loop with carried dependency and multiple operations */
    for (i = 0; i < size; ++i) {
        /* Complex operation chain with true data dependencies:
         * 1. sum * a[i] creates dependency on previous sum
         * 2. + b[i] adds another operation
         * 3. >> 1 creates third operation
         * This creates a recurrence cycle that challenges the scheduler */
        sum = (sum * a[i] + b[i]) >> 1;
        
        /* Additional operations to increase instruction count */
        sum ^= (a[i] & 0xFF);  /* XOR operation */
        sum += (b[i] % 16);    /* Modulo operation */
    }
    
    return sum;
}

/* Outer loop to provide multiple scheduling contexts */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int main(void) {
    volatile int seed = time(NULL);  /* Prevent constant propagation */
    srand(seed);
    
    int a[128], b[128];
    int i, j;
    int total_sum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 128; ++i) {
        a[i] = rand() % 100 + 1;  /* Non-zero values */
        b[i] = rand() % 100 + 1;
    }
    
    /* Outer loop - provides multiple contexts for scheduling analysis */
    for (j = 0; j < 10; ++j) {
        /* Call the inner loop computation */
        int result = compute_loop(a, b, 32);  /* Fixed small iteration count */
        total_sum += result;
        
        /* Modify input arrays slightly to create loop-variant behavior */
        b[0] += result & 0xF;  /* Small modification based on result */
        a[j % 32] ^= result;   /* Modify different elements each outer iteration */
        
        /* Additional computation to prevent optimization */
        for (i = 0; i < 8; ++i) {
            a[i] = (a[i] * 1103515245 + 12345) & 0x7FFFFFFF;
        }
    }
    
    printf("Final result: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;  /* Use result to prevent dead code elimination */
}

/* Program to trigger GCC modulo scheduling debug output in modulo-sched.cc */
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
        sum = sum + (b[i] % 16);
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
        
        /* Modify input slightly to create loop-variant behavior */
        a[0] += j;
        b[0] += result;
    }
    
    return total;
}

int main(void) {
    const int ARRAY_SIZE = 64;
    const int INNER_LOOP_SIZE = 32;  /* Small, constant trip count */
    const int OUTER_LOOP_ITERS = 10;
    
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    int i;
    
    /* Initialize with pseudo-random values to prevent constant propagation */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile int seed = rand();
    a[0] += seed;
    b[0] += seed;
    
    /* Execute the nested loop structure */
    int final_result = outer_loop(a, b, INNER_LOOP_SIZE, OUTER_LOOP_ITERS);
    
    /* Print result to prevent elimination */
    printf("Final result: %d\n", final_result);
    
    return 0;
}

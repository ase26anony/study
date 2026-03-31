/* Program to trigger GCC modulo scheduling debug output in modulo-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling to focus on modulo scheduling */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
static int compute_loop(int *a, int *b, int size) {
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

/* Outer wrapper to create multiple scheduling contexts */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int main(void) {
    volatile int seed = time(NULL);  /* Prevent constant propagation */
    srand(seed);
    
    int a[128], b[128];
    int i, j;
    int total_sum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 128; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Outer loop - provides multiple contexts for scheduling analysis */
    for (j = 0; j < 10; ++j) {
        /* Inner loop with carried dependency - modulo scheduling candidate */
        int loop_result = compute_loop(a, b, 32);  /* Fixed small trip count */
        total_sum += loop_result;
        
        /* Modify input slightly to create loop-variant behavior */
        b[0] += loop_result % 100;
        a[j % 32] = total_sum & 0xFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}

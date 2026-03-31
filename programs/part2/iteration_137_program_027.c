/* Program to trigger modulo scheduling debug output in GCC */
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
        sum = sum + (b[i] % 16);
        
        /* Another carried dependency */
        a[i] = (a[i] + sum) & 0x7FFF;
    }
    
    return sum;
}

/* Outer loop to provide multiple scheduling contexts */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int outer_loop(int *a, int *b, int outer_iter, int inner_size) {
    int total = 0;
    int j;
    
    for (j = 0; j < outer_iter; ++j) {
        /* Call the inner computation */
        int result = compute_loop(a, b, inner_size);
        total += result;
        
        /* Modify input slightly to create loop-variant behavior */
        b[0] += result & 0xF;
        a[j % inner_size] ^= total;
    }
    
    return total;
}

int main(void) {
    const int ARRAY_SIZE = 64;  /* Small, compile-time constant */
    const int OUTER_ITER = 10;  /* Small outer loop count */
    
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    volatile int seed = time(NULL);  /* Prevent constant propagation */
    
    srand(seed);
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Execute the nested loops */
    int final_result = outer_loop(a, b, OUTER_ITER, ARRAY_SIZE);
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d\n", final_result);
    
    return 0;
}

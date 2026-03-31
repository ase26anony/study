/* Compile with: gcc -O2 -fmodulo-sched -fno-tree-vectorize -fno-unroll-loops -fdump-rtl-sms -fdump-rtl-sms-details modulo_test.c -o modulo_test */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function with attributes to prevent unwanted optimizations */
__attribute__((noinline, optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop(int* a, int* b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i;
    
    /* Inner loop with carried dependency - critical for modulo scheduling */
    for (i = 0; i < size; ++i) {
        /* Complex operation with true data dependency across iterations */
        sum = (sum * a[i] + b[i]) >> 1;
        
        /* Additional operations to increase instruction count */
        sum = sum ^ (a[i] & 0xFF);
        sum = sum + (b[i] % 256);
    }
    
    return sum;
}

/* Outer loop to provide multiple scheduling contexts */
__attribute__((noinline, optimize("no-tree-vectorize", "no-unroll-loops")))
int outer_loop(int* a, int* b, int outer_iter, int inner_size) {
    int total = 0;
    int j;
    
    for (j = 0; j < outer_iter; ++j) {
        /* Call inner computation */
        int result = compute_loop(a, b, inner_size);
        total += result;
        
        /* Modify input slightly to create loop-variant behavior */
        a[0] += j;
        b[0] += result;
    }
    
    return total;
}

int main() {
    const int ARRAY_SIZE = 64;
    const int OUTER_ITER = 10;
    const int INNER_SIZE = 32;  /* Small, constant trip count */
    
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
    int final_result = outer_loop(a, b, OUTER_ITER, INNER_SIZE);
    
    /* Print result to prevent elimination */
    printf("Final result: %d\n", final_result);
    
    return 0;
}

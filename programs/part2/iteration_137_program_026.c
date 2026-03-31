/* Program to trigger modulo scheduling debug logging in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    volatile int temp;  /* Prevent optimizations */
    
    /* Outer loop to increase scheduling analysis opportunities */
    for (int outer = 0; outer < 10; outer++) {
        /* Critical inner loop with carried dependency */
        /* Fixed small iteration count for modulo scheduling */
        for (int i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations */
            /* sum depends on previous sum value - creates true dependency */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            sum = sum + (b[i] % 16);
        }
        
        /* Modify input slightly to create loop-variant behavior */
        /* This prevents the outer loop from being optimized away */
        b[0] += sum;
        
        /* Use volatile to prevent dead code elimination */
        temp = sum;
    }
    
    return sum;
}

int main() {
    const int ARRAY_SIZE = 128;
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    
    /* Initialize with pseudo-random values */
    /* Using volatile source to prevent constant propagation */
    volatile int seed = time(NULL);
    srand(seed);
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    /* Call the test function multiple times */
    int total = 0;
    for (int iter = 0; iter < 3; iter++) {
        total += modulo_sched_test(a, b, ARRAY_SIZE);
        
        /* Modify arrays between calls */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            a[i] = (a[i] + 1) % 256;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}

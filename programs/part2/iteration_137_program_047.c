/* Program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
void modulo_sched_loop(int *a, int *b, int n) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    volatile int v = 0;  /* Volatile to prevent optimizations */
    
    /* Outer loop to increase scheduling analysis opportunities */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        for (int i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum ^= (a[i] & 0xFF);
            sum += (b[i] % 17);
        }
        
        /* Modify input slightly to create loop-variant behavior */
        b[0] += sum;
        a[outer % 32] ^= sum;
        
        /* Use volatile to prevent dead code elimination */
        v = sum;
    }
    
    /* Use result to prevent elimination */
    printf("Final sum: %d\n", sum);
}

int main() {
    const int ARRAY_SIZE = 128;
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    
    /* Initialize with pseudo-random values to prevent constant propagation */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call the function with carried dependency loop */
    modulo_sched_loop(a, b, ARRAY_SIZE);
    
    return 0;
}

/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Use restrict to guarantee no aliasing for better dependence analysis */
static void process_loops(int *restrict data1, int *restrict data2, 
                          int *restrict sum1, int *restrict sum2, 
                          volatile int outer_bound) {
    volatile int side_effect = 0;
    
    /* Outer loop with volatile bound to prevent full unrolling */
    for (int j = 0; j < outer_bound; ++j) {
        /* Reset accumulators each outer iteration */
        int acc1 = 0;
        int acc2 = 0;
        
        /* Inner loop - target for modulo scheduling */
        /* Multiple recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            acc1 = data1[i] + acc1;  /* distance=1 dependency */
            
            /* Second recurrence: mixed operations */
            acc2 = (data2[i] * 3) - acc2;  /* Another distance=1 dependency */
            
            /* Third recurrence: more complex operation */
            /* This creates additional dependency edges */
            if (i > 0) {
                data1[i] = data1[i-1] + 1;  /* Explicit distance=1 store */
            }
        }
        
        /* Store results with side effect to prevent elimination */
        *sum1 += acc1;
        *sum2 += acc2;
        side_effect = j;  /* Prevent dead code elimination */
    }
}

int main(void) {
    /* Initialize with volatile to prevent constant propagation */
    volatile int outer_bound = 100;
    
    /* Allocate and initialize arrays */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
    }
    
    /* Accumulation variables */
    int sum1 = 0;
    int sum2 = 0;
    
    /* Process the loops */
    process_loops(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Use results to prevent optimization */
    printf("Final sums: %d, %d\n", sum1, sum2);
    
    /* Cleanup */
    free(data1);
    free(data2);
    
    return 0;
}

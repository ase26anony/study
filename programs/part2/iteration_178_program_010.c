/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function with restrict to avoid aliasing issues */
static void compute_recurrences(int *restrict data1, int *restrict data2, 
                                int *restrict sum1, int *restrict sum2, 
                                volatile int outer_bound) {
    volatile int side_effect = 0;
    
    /* Outer loop to provide multiple contexts */
    for (int j = 0; j < outer_bound; ++j) {
        int local_sum1 = *sum1;
        int local_sum2 = *sum2;
        
        /* Inner loop - target for modulo scheduling */
        /* Multiple independent recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: mixed operations to create different dependencies */
            local_sum2 = (data2[i] * 3) - local_sum2;
            
            /* Third recurrence: more complex operation chain */
            /* Creates additional dependency edges */
            local_sum1 = local_sum1 + (data1[INNER_ITER - i - 1] >> 1);
        }
        
        /* Update outer sums */
        *sum1 = local_sum1;
        *sum2 = local_sum2;
        
        /* Side effect to prevent dead code elimination */
        side_effect = j;
    }
    
    /* Use side effect to ensure it's not optimized away */
    if (side_effect < 0) {
        printf("Impossible branch\n");
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
    
    /* Seed RNG for initialization */
    srand(time(NULL));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
    }
    
    /* Accumulation variables */
    int sum1 = 0;
    int sum2 = 0;
    
    /* Perform computation */
    compute_recurrences(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Use results to prevent optimization */
    printf("Results: sum1 = %d, sum2 = %d\n", sum1, sum2);
    
    /* Cleanup */
    free(data1);
    free(data2);
    
    return 0;
}

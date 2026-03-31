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
        int local_sum1 = 0;
        int local_sum2 = 0;
        
        /* INNER LOOP - Target for modulo scheduling */
        /* Constant small bound, multiple recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            /* Creates distance=1 dependency: local_sum1[i] depends on local_sum1[i-1] */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: multiplication-accumulation with different latency */
            /* Creates another distance=1 dependency chain */
            local_sum2 = data2[i] * 3 + local_sum2;
            
            /* Third recurrence: mixed operations to create more edges */
            /* This creates cross-iteration dependency through data1 */
            data1[i] = data1[i] - local_sum1 / 2;
        }
        
        /* Prevent dead code elimination */
        side_effect = local_sum1 + local_sum2;
        
        /* Accumulate to outer sums */
        *sum1 += local_sum1;
        *sum2 += local_sum2;
    }
    
    /* Use side_effect to prevent optimization */
    if (side_effect == 0x1234) {
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
    
    /* Print results to prevent optimization */
    printf("Final sums: sum1 = %d, sum2 = %d\n", sum1, sum2);
    
    /* Cleanup */
    free(data1);
    free(data2);
    
    return 0;
}

/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function with restrict to guarantee no aliasing */
void process_loops(int *restrict data1, int *restrict data2, 
                   int *restrict sum1, int *restrict sum2, 
                   int outer_bound) {
    volatile int side_effect = 0;  /* Prevent dead code elimination */
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Reset accumulators each outer iteration */
        int local_sum1 = 0;
        int local_sum2 = 0;
        
        /* Inner loop with multiple recurrences - target for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple accumulation */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: more complex operation with different latency */
            local_sum2 = data2[i] * 3 + local_sum2;
            
            /* Third recurrence: mixed operation to create more edges */
            local_sum1 = local_sum1 - (data1[i] >> 2);
        }
        
        /* Side effect to prevent outer loop elimination */
        side_effect = local_sum1 + local_sum2;
        
        /* Accumulate to output parameters */
        *sum1 += local_sum1;
        *sum2 += local_sum2;
    }
}

int main() {
    /* Use volatile to prevent constant propagation */
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
    int total_sum1 = 0;
    int total_sum2 = 0;
    
    /* Process the loops */
    process_loops(data1, data2, &total_sum1, &total_sum2, outer_bound);
    
    /* Use results to prevent optimization */
    printf("Result sum1: %d\n", total_sum1);
    printf("Result sum2: %d\n", total_sum2);
    
    /* Cleanup */
    free(data1);
    free(data2);
    
    return 0;
}

/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function containing the target loop structure */
static void process_arrays(int *restrict data1, int *restrict data2, 
                          int *restrict sum1, int *restrict sum2, 
                          int outer_bound) {
    volatile int side_effect = 0;
    
    /* Outer loop with runtime bound to prevent unrolling */
    for (int j = 0; j < outer_bound; ++j) {
        /* Reset accumulators each outer iteration */
        int local_sum1 = 0;
        int local_sum2 = 0;
        
        /* INNER LOOP: Target for modulo scheduling
         * Contains multiple recurrences with loop-carried dependencies
         */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with addition */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: different operation (subtract) */
            local_sum2 = local_sum2 - data2[i];
            
            /* Third recurrence: multiplication chain */
            if (i > 0) {
                /* Create distance-1 dependency */
                data1[i] = data1[i-1] * 3;
            }
        }
        
        /* Accumulate results and create side effect */
        *sum1 += local_sum1;
        *sum2 += local_sum2;
        side_effect = j;  /* Prevent dead code elimination */
    }
}

int main(void) {
    /* Use volatile for outer bound to prevent constant propagation */
    volatile int outer_bound = 100;
    int actual_bound = outer_bound;
    
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
    
    /* Process the arrays */
    process_arrays(data1, data2, &total_sum1, &total_sum2, actual_bound);
    
    /* Use results to prevent optimization */
    printf("Result sum1: %d\n", total_sum1);
    printf("Result sum2: %d\n", total_sum2);
    
    /* Cleanup */
    free(data1);
    free(data2);
    
    return 0;
}

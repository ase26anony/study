/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler
 * Specifically targets the print_node_edges function in modulo-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo-schedulable loop */
#define ARRAY_SIZE 1024

/* Force compiler to consider dependencies */
static void process_loops(int *restrict data1, int *restrict data2, 
                          int *restrict sum1, int *restrict sum2, 
                          int outer_bound) {
    volatile int outer_counter = 0;  /* Prevent outer loop optimization */
    int i, j;
    
    /* Outer loop with runtime bound */
    for (j = 0; j < outer_bound; ++j) {
        /* Reset accumulators each outer iteration */
        int local_sum1 = 0;
        int local_sum2 = 0;
        
        /* Target inner loop - should be modulo-scheduled */
        /* Multiple independent recurrences with different operations */
        for (i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            local_sum1 = data1[i] + local_sum1;  /* distance=1 dependency */
            
            /* Second recurrence: mixed operations */
            local_sum2 = (data2[i] * 3) - local_sum2;  /* different latency */
            
            /* Third recurrence: more complex dependency chain */
            if (i > 0) {
                data1[i] = data1[i-1] + data1[i];  /* true loop-carried dep */
            }
        }
        
        /* Update global sums */
        *sum1 += local_sum1;
        *sum2 += local_sum2;
        
        /* Prevent dead code elimination */
        outer_counter = j;
    }
    
    /* Use outer_counter to prevent optimization */
    (void)outer_counter;
}

/* Alternative version with different recurrence patterns */
static void process_loops_variant(int *restrict arr1, int *restrict arr2,
                                  int *restrict acc1, int *restrict acc2,
                                  int outer_bound) {
    volatile int dummy = 0;
    int j, k;
    
    for (j = 0; j < outer_bound; ++j) {
        int temp1 = *acc1;
        int temp2 = *acc2;
        
        /* Inner loop with multiple dependency types */
        for (k = 0; k < INNER_ITER; ++k) {
            /* Chain 1: accumulation with multiplication */
            temp1 = arr1[k] * temp1 + k;
            
            /* Chain 2: accumulation with subtraction */
            temp2 = arr2[k] - temp2;
            
            /* Chain 3: data recurrence */
            if (k < INNER_ITER - 1) {
                arr1[k+1] = arr1[k] + arr2[k];
            }
        }
        
        *acc1 = temp1;
        *acc2 = temp2;
        dummy = j;  /* Anti-optimization */
    }
    
    (void)dummy;
}

int main(void) {
    int *data1, *data2;
    int sum1 = 0, sum2 = 0;
    int acc1 = 0, acc2 = 0;
    volatile int outer_bound = 100;  /* Prevent constant propagation */
    int i;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
    data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
    }
    
    /* Process with first loop pattern */
    process_loops(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Process with variant pattern */
    process_loops_variant(data1, data2, &acc1, &acc2, outer_bound / 2);
    
    /* Combine results to ensure computation isn't optimized away */
    int final_result = sum1 + sum2 + acc1 + acc2;
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(data1);
    free(data2);
    
    return 0;
}

/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler
 * Specifically targets the print_node_edges function in modulo-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant bound for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function with restrict qualifier to ensure no aliasing */
void process_loops(int *restrict data1, int *restrict data2, 
                   int *restrict sum1, int *restrict sum2, 
                   int outer_bound) {
    volatile int side_effect = 0;  /* Prevent dead code elimination */
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Reset accumulators each outer iteration */
        int local_sum1 = 0;
        int local_sum2 = 0;
        
        /* TARGET INNER LOOP - Should be modulo scheduled */
        /* Contains multiple recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            local_sum1 = data1[i] + local_sum1;  /* distance=1 dependency */
            
            /* Second recurrence: mixed operations */
            local_sum2 = (data2[i] * 3) - local_sum2;  /* different latency */
            
            /* Third recurrence: more complex dependency chain */
            if (i > 0) {
                data1[i] = data1[i-1] + data1[i];  /* true loop-carried dep */
            }
        }
        
        /* Side effect to prevent outer loop elimination */
        side_effect = local_sum1 + local_sum2;
        
        /* Accumulate to global sums */
        *sum1 += local_sum1;
        *sum2 += local_sum2;
    }
}

/* Alternative implementation with different recurrence patterns */
void process_loops_variant(int *restrict arr1, int *restrict arr2,
                          int *restrict sum1, int *restrict sum2,
                          int outer_bound) {
    volatile int counter = 0;
    
    for (int j = 0; j < outer_bound; ++j) {
        int acc1 = *sum1;
        int acc2 = *sum2;
        
        /* Multiple independent recurrences in same loop */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* Recurrence 1: accumulation with multiplication */
            acc1 = arr1[i] * 2 + acc1;
            
            /* Recurrence 2: accumulation with subtraction */
            acc2 = arr2[i] - acc2;
            
            /* Recurrence 3: shift-based operation */
            if (i > 0) {
                arr1[i] = (arr1[i-1] << 1) | arr1[i];
            }
        }
        
        counter += acc1 + acc2;
        *sum1 = acc1;
        *sum2 = acc2;
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
    int sum1 = 0;
    int sum2 = 0;
    
    /* Process loops - this should trigger modulo scheduling */
    process_loops(data1, data2, &sum1, &sum2, actual_bound);
    
    /* Also try the variant */
    int sum3 = 0, sum4 = 0;
    process_loops_variant(data1, data2, &sum3, &sum4, actual_bound / 2);
    
    /* Use results to prevent optimization */
    printf("Results: %d %d %d %d\n", sum1, sum2, sum3, sum4);
    
    free(data1);
    free(data2);
    
    return 0;
}

/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler
 * Specifically targets the print_node_edges function in modulo-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Use restrict to prevent aliasing analysis issues */
static void process_loops(int *restrict data1, int *restrict data2, 
                         int *restrict sum1, int *restrict sum2, 
                         volatile int outer_bound) {
    int i, j;
    
    /* Outer loop with volatile bound to prevent unrolling */
    for (j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - candidate for modulo scheduling */
        /* Multiple independent recurrences to create dependency edges */
        for (i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with addition */
            *sum1 = data1[i] + *sum1;
            
            /* Second recurrence: different operation (multiply-add) */
            *sum2 = data2[i] * 3 + *sum2;
            
            /* Third recurrence: mixed operations to create varied edges */
            data1[i] = data1[i] + (*sum1 >> 2);  /* Use shift for different latency */
        }
        
        /* Prevent dead code elimination without breaking modulo scheduling */
        volatile int sink = *sum1 + *sum2;
        (void)sink;  /* Suppress unused variable warning */
        
        /* Small perturbation to prevent complete optimization */
        data1[0] += j;
        data2[0] -= j;
    }
}

/* Alternative loop structure with array-based recurrence */
static void process_with_array_recurrence(int *restrict arr1, int *restrict arr2,
                                         volatile int outer_bound) {
    int i, j;
    
    for (j = 0; j < outer_bound; ++j) {
        /* Loop-carried dependency through array */
        for (i = 1; i < INNER_ITER; ++i) {
            /* True recurrence: arr1[i] depends on arr1[i-1] */
            arr1[i] = arr1[i-1] * 2 + arr2[i];
            
            /* Independent recurrence in second array */
            arr2[i] = arr2[i-1] + arr1[i] / 4;
        }
        
        /* Cross-iteration dependency to create distance edges */
        arr1[0] = arr1[INNER_ITER-1] + 1;
        arr2[0] = arr2[INNER_ITER-1] - 1;
    }
}

int main(void) {
    int *data1, *data2;
    int sum1 = 0, sum2 = 0;
    volatile int outer_bound = 100;  /* Volatile to prevent constant propagation */
    
    /* Seed RNG for reproducible array values */
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
    data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
    }
    
    /* First test: reduction-style recurrences */
    process_loops(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Second test: array-based recurrences */
    process_with_array_recurrence(data1 + 100, data2 + 100, outer_bound / 2);
    
    /* Use results to prevent optimization */
    printf("Results: sum1=%d, sum2=%d, data1[0]=%d, data2[0]=%d\n",
           sum1, sum2, data1[0], data2[0]);
    
    free(data1);
    free(data2);
    
    return 0;
}

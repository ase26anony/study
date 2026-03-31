/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler
 * Specifically targets the print_node_edges function in modulo-sched.cc
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
    volatile int sink = 0;  /* Prevent dead code elimination */
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - prime candidate for modulo scheduling */
        /* Multiple independent recurrences to create multiple dependency edges */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple accumulation (distance=1 dependency) */
            *sum1 = data1[i] + *sum1;
            
            /* Second recurrence: different operation with different latency */
            *sum2 = data2[i] * *sum2 + 1;
            
            /* Third recurrence: mixed operations to create varied edges */
            data1[i] = data1[i] - *sum1;
            
            /* Fourth recurrence: another independent chain */
            data2[i] = data2[i] + *sum2 * 2;
        }
        
        /* Prevent outer loop elimination */
        sink = *sum1 + *sum2;
    }
}

/* Alternative version with more complex recurrences */
static void process_loops_v2(int *restrict arr1, int *restrict arr2,
                            int *restrict acc1, int *restrict acc2,
                            volatile int outer_bound) {
    volatile int dummy = 0;
    
    for (int k = 0; k < outer_bound; ++k) {
        /* Inner loop designed specifically for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* Multiple loop-carried dependencies with distance=1 */
            
            /* Type 1: Simple reduction */
            *acc1 += arr1[i];
            
            /* Type 2: Recurrence with multiplication (different latency) */
            *acc2 = arr2[i] * *acc2;
            
            /* Type 3: Array recurrence - value depends on previous iteration */
            if (i > 0) {
                arr1[i] = arr1[i-1] + arr2[i];
            }
            
            /* Type 4: Another independent accumulation */
            arr2[i] = arr2[i] + *acc1;
        }
        
        dummy = *acc1 ^ *acc2;  /* Use result to prevent elimination */
    }
}

int main(void) {
    /* Volatile to prevent constant propagation and unrolling */
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
    
    /* Accumulators - must be initialized */
    int sum1 = 0;
    int sum2 = 1;  /* Start with 1 for multiplicative recurrence */
    
    /* Call the loop processing function */
    process_loops(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Also test the second version */
    int acc1 = 0, acc2 = 1;
    process_loops_v2(data1, data2, &acc1, &acc2, outer_bound);
    
    /* Use results to prevent optimization */
    printf("Results: sum1=%d, sum2=%d, acc1=%d, acc2=%d\n", 
           sum1, sum2, acc1, acc2);
    
    free(data1);
    free(data2);
    
    return 0;
}

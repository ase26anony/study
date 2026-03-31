/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function containing the target loop structure */
static void compute_sums(int *restrict data1, int *restrict data2, 
                         int *restrict sum1, int *restrict sum2, 
                         volatile int outer_bound) {
    int local_sum1 = 0;
    int local_sum2 = 0;
    
    /* Outer loop with volatile bound to prevent full optimization */
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - target for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with loop-carried dependency */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: different operation to create separate dependency chain */
            local_sum2 = data2[i] * 3 + local_sum2;
            
            /* Third recurrence: mixed operations to increase edge complexity */
            local_sum1 = local_sum1 - (data1[i] >> 2);
        }
        
        /* Prevent dead code elimination without inhibiting modulo scheduling */
        volatile int dummy = local_sum1;
        (void)dummy;
        
        /* Small side effect to prevent outer loop removal */
        data1[0] = j;
    }
    
    *sum1 = local_sum1;
    *sum2 = local_sum2;
}

/* Alternative loop structure with multiple independent recurrences */
static void compute_multiple_recurrences(int *restrict arr1, int *restrict arr2,
                                         int *restrict results, volatile int outer_bound) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with multiple dependency chains */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* Chain 1: Simple additive recurrence */
            acc1 = arr1[i] + acc1;
            
            /* Chain 2: Multiplicative recurrence (creates different latency) */
            acc2 = arr2[i] * acc2 + 1;
            
            /* Chain 3: Mixed arithmetic with shift */
            acc3 = (acc3 << 1) + arr1[i];
            
            /* Chain 4: Another additive chain with different source */
            acc1 = acc1 + arr2[i];
        }
        
        /* Prevent optimization */
        volatile int marker = acc1 + acc2;
        (void)marker;
    }
    
    results[0] = acc1;
    results[1] = acc2;
    results[2] = acc3;
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
    
    int sum1 = 0, sum2 = 0;
    
    /* Call the function with the target loop structure */
    compute_sums(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Second test case with more complex recurrences */
    int results[3];
    compute_multiple_recurrences(data1, data2, results, outer_bound / 2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: sum1 = %d, sum2 = %d\n", sum1, sum2);
    printf("Multiple recurrences: %d, %d, %d\n", results[0], results[1], results[2]);
    
    free(data1);
    free(data2);
    
    return 0;
}

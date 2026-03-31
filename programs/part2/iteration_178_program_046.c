/* modulo-sched-coverage.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function containing the target loop structure */
void compute_sums(int *restrict data1, int *restrict data2, 
                  int *restrict sum1, int *restrict sum2, 
                  volatile int outer_bound) {
    int local_sum1 = 0;
    int local_sum2 = 0;
    
    /* Outer loop with volatile bound to prevent full optimization */
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - target for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with addition */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: different operation (multiply-add) */
            local_sum2 = data2[i] * 3 + local_sum2;
            
            /* Third recurrence: mixed operations to create more edges */
            local_sum1 = local_sum1 - (data1[i] >> 2);
        }
        
        /* Prevent dead code elimination without inhibiting modulo scheduling */
        volatile int dummy = local_sum1;
        (void)dummy;
        
        /* Small side effect to prevent outer loop removal */
        data1[0] = j % 256;
    }
    
    *sum1 = local_sum1;
    *sum2 = local_sum2;
}

/* Another function with different recurrence patterns */
void compute_more_sums(int *restrict arr1, int *restrict arr2,
                       int *restrict result1, int *restrict result2,
                       volatile int iterations) {
    int acc1 = 0;
    int acc2 = 1;  /* Start with 1 for multiplicative recurrence */
    
    for (int k = 0; k < iterations; ++k) {
        /* Inner loop with multiple independent recurrences */
        for (int i = 0; i < 16; ++i) {  /* Different constant for variety */
            /* Simple additive recurrence */
            acc1 = arr1[i] + acc1;
            
            /* Multiplicative recurrence (creates different latency pattern) */
            acc2 = arr2[i] * acc2;
            
            /* Another additive recurrence with different array */
            acc1 = acc1 + (arr2[i] & 0xFF);
        }
        
        /* Prevent optimization */
        volatile int marker = acc2;
        (void)marker;
    }
    
    *result1 = acc1;
    *result2 = acc2;
}

int main(void) {
    /* Initialize with volatile to prevent constant propagation */
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
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
    }
    
    int sum1 = 0, sum2 = 0;
    int res1 = 0, res2 = 0;
    
    /* Call first computation function */
    compute_sums(data1, data2, &sum1, &sum2, actual_bound);
    
    /* Call second function with different parameters */
    compute_more_sums(data1 + 64, data2 + 64, &res1, &res2, actual_bound / 2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: sum1 = %d, sum2 = %d, res1 = %d, res2 = %d\n", 
           sum1, sum2, res1, res2);
    
    free(data1);
    free(data2);
    
    return 0;
}

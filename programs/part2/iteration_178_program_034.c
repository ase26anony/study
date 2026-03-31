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
void compute_sums(int *restrict data1, int *restrict data2, 
                  int *restrict sum1, int *restrict sum2, 
                  int outer_bound) {
    volatile int side_effect = 0;  /* Prevent dead code elimination */
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Reset accumulators each outer iteration */
        int local_sum1 = 0;
        int local_sum2 = 0;
        
        /* TARGET INNER LOOP - Candidate for modulo scheduling */
        /* Multiple recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: mixed operations with multiplication */
            local_sum2 = (data2[i] * 3) - local_sum2;
            
            /* Third independent recurrence for more edges */
            /* Using different array indices to ensure separate dependencies */
            if (i > 0) {
                data1[i] = data1[i-1] + 7;  /* Simple recurrence pattern */
            }
        }
        
        /* Side effect to prevent outer loop elimination */
        side_effect = local_sum1 + local_sum2;
        
        /* Accumulate to output parameters */
        *sum1 += local_sum1;
        *sum2 += local_sum2;
    }
}

/* Alternative implementation with different recurrence patterns */
void compute_mixed(int *restrict arr1, int *restrict arr2, 
                   int *restrict result1, int *restrict result2,
                   int outer_bound) {
    volatile int counter = 0;
    
    for (int j = 0; j < outer_bound; ++j) {
        int acc1 = *result1;
        int acc2 = *result2;
        
        /* Inner loop with multiple dependency chains */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* Chain 1: Multiply-accumulate with loop-carried dependency */
            acc1 = arr1[i] * acc1 + i;
            
            /* Chain 2: Add-subtract pattern */
            acc2 = arr2[i] - acc2 + 5;
            
            /* Chain 3: Simple shift-based recurrence */
            if (i % 2 == 0) {
                arr1[i] = (arr1[i] << 1) | 1;
            }
        }
        
        counter += acc1 + acc2;
        *result1 = acc1;
        *result2 = acc2;
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
    
    /* Accumulators */
    int sum1 = 0, sum2 = 0;
    int res1 = 1, res2 = 1;  /* Non-zero initial values for multiplications */
    
    /* Call both functions to increase scheduling opportunities */
    compute_sums(data1, data2, &sum1, &sum2, actual_bound);
    compute_mixed(data1 + 64, data2 + 64, &res1, &res2, actual_bound / 2);
    
    /* Use results to prevent optimization */
    printf("Results: sum1=%d, sum2=%d, res1=%d, res2=%d\n", 
           sum1, sum2, res1, res2);
    
    /* Cleanup */
    free(data1);
    free(data2);
    
    return 0;
}

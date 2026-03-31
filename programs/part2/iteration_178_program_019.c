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
                         int outer_bound) {
    volatile int outer_counter = 0;  /* Prevent outer loop optimization */
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Reset accumulators each outer iteration */
        int local_sum1 = 0;
        int local_sum2 = 0;
        
        /* TARGET INNER LOOP - Candidate for modulo scheduling */
        /* Contains multiple recurrences with loop-carried dependencies */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple accumulation */
            local_sum1 = data1[i] + local_sum1;  /* distance=1 dependency */
            
            /* Second recurrence: mixed operation with different latency */
            local_sum2 = data2[i] * 3 + local_sum2;  /* Another distance=1 dependency */
            
            /* Third independent recurrence to increase edge count */
            /* Using subtraction for different operation type */
            if (i > 0) {
                local_sum1 = local_sum1 - (data1[i-1] >> 2);  /* Additional dependency */
            }
        }
        
        /* Update global sums */
        *sum1 += local_sum1;
        *sum2 += local_sum2;
        
        /* Side effect to prevent dead code elimination */
        outer_counter = j;
    }
}

/* Alternative implementation with more complex recurrence patterns */
static void compute_sums_variant(int *restrict arr1, int *restrict arr2,
                                 int *restrict result1, int *restrict result2,
                                 int iterations) {
    volatile int dummy = 0;
    
    for (int outer = 0; outer < iterations; ++outer) {
        int acc1 = arr1[0];
        int acc2 = arr2[0];
        
        /* Inner loop with multiple interleaved recurrences */
        for (int i = 1; i < INNER_ITER; ++i) {
            /* Chain 1: Linear recurrence */
            acc1 = arr1[i] * 2 + acc1;  /* Multiplication has different latency */
            
            /* Chain 2: Another independent recurrence */
            acc2 = arr2[i] - acc2;  /* Subtraction operation */
            
            /* Chain 3: More complex dependency with shift */
            if (i > 1) {
                acc1 = acc1 + (arr1[i-2] << 1);  /* Distance=2 dependency */
            }
        }
        
        *result1 ^= acc1;  /* Use XOR to combine results */
        *result2 ^= acc2;
        
        dummy = outer;  /* Prevent optimization */
    }
}

int main(void) {
    /* Initialize with volatile source to prevent constant propagation */
    volatile int outer_bound_source = 100;
    int outer_bound = outer_bound_source;
    
    /* Allocate and initialize arrays */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    srand(time(NULL));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 256;
        data2[i] = rand() % 256;
    }
    
    int total_sum1 = 0;
    int total_sum2 = 0;
    int result1 = 0;
    int result2 = 0;
    
    /* Call both implementations to increase scheduling opportunities */
    compute_sums(data1, data2, &total_sum1, &total_sum2, outer_bound);
    compute_sums_variant(data1, data2, &result1, &result2, outer_bound / 2);
    
    /* Combine and use results to prevent optimization */
    int final_result = total_sum1 + total_sum2 + result1 + result2;
    
    printf("Result: %d\n", final_result);
    
    free(data1);
    free(data2);
    
    return 0;
}

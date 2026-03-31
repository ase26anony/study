/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function with restrict to guarantee no aliasing */
void compute_sums(int *restrict data1, int *restrict data2, 
                  int *restrict sum1, int *restrict sum2, 
                  int outer_bound) {
    volatile int outer_volatile = outer_bound; /* Prevent optimization */
    int local_sum1 = 0, local_sum2 = 0;
    
    /* Outer loop - prevents full unrolling of inner loop */
    for (int j = 0; j < outer_volatile; ++j) {
        /* Inner loop - constant small bound, ideal for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum1 += data1[i] (accumulation) */
            local_sum1 += data1[i];
            
            /* Second recurrence: sum2 += data2[i] * k (with multiplication) */
            int k = 3; /* Constant multiplier for different latency */
            local_sum2 += data2[i] * k;
            
            /* Third recurrence: mixed operation chain */
            local_sum1 = local_sum1 - (data2[i] >> 2); /* Different operation mix */
        }
        
        /* Side effect to prevent dead code elimination */
        volatile int side_effect = local_sum1 + local_sum2;
        (void)side_effect; /* Suppress unused warning */
        
        /* Slight permutation to create varied dependencies */
        if (j % 2 == 0) {
            local_sum2 += data1[j % INNER_ITER];
        }
    }
    
    *sum1 = local_sum1;
    *sum2 = local_sum2;
}

/* Alternative implementation with multiple independent recurrences */
void compute_multiple_recurrences(int *restrict arr1, int *restrict arr2,
                                  int *restrict res1, int *restrict res2,
                                  int outer_bound) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    volatile int bound = outer_bound;
    
    for (int j = 0; j < bound; ++j) {
        /* Inner loop with multiple independent dependency chains */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* Chain 1: Simple accumulation */
            acc1 += arr1[i];
            
            /* Chain 2: Multiplication accumulation */
            acc2 += arr2[i] * 2;
            
            /* Chain 3: Recurrence with data dependency between iterations */
            int temp = acc3 + arr1[i];
            acc3 = temp - arr2[i]; /* Creates more complex dependencies */
            
            /* Chain 4: Shift operation with different latency */
            acc1 ^= (arr1[i] << 1);
        }
        
        /* Prevent optimization */
        volatile int marker = acc1 + acc2 + acc3;
        (void)marker;
    }
    
    *res1 = acc1;
    *res2 = acc2;
}

int main(void) {
    /* Initialize with volatile to prevent constant propagation */
    volatile int outer_bound = 100;
    int sum1 = 0, sum2 = 0;
    int res1 = 0, res2 = 0;
    
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
    
    /* Call the computation functions */
    compute_sums(data1, data2, &sum1, &sum2, outer_bound);
    compute_multiple_recurrences(data1, data2, &res1, &res2, outer_bound);
    
    /* Use results to prevent optimization */
    printf("Results: sum1=%d, sum2=%d, res1=%d, res2=%d\n", 
           sum1, sum2, res1, res2);
    
    free(data1);
    free(data2);
    
    return 0;
}

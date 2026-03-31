/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function with inner loop containing multiple recurrences */
static void process_arrays(int *restrict data1, int *restrict data2, 
                          int *restrict sum1, int *restrict sum2, 
                          int outer_iter) {
    volatile int outer_counter = outer_iter; /* Prevent optimization */
    int local_sum1 = *sum1;
    int local_sum2 = *sum2;
    
    /* Outer loop - prevents full unrolling of inner loop */
    for (int j = 0; j < outer_counter; ++j) {
        /* Inner loop with constant bound - candidate for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with loop-carried dependency */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: different operation to create separate dependency chain */
            local_sum2 = data2[i] * local_sum2 + i;
            
            /* Third recurrence: mixed operations to increase edge complexity */
            data1[i] = data1[i] - local_sum1 / 2;
        }
        
        /* Prevent dead code elimination */
        volatile int side_effect = local_sum1 + local_sum2;
        (void)side_effect;
        
        /* Shift arrays to create varying patterns */
        int temp1 = data1[0];
        int temp2 = data2[0];
        for (int k = 0; k < INNER_ITER - 1; ++k) {
            data1[k] = data1[k + 1];
            data2[k] = data2[k + 1];
        }
        data1[INNER_ITER - 1] = temp1;
        data2[INNER_ITER - 1] = temp2;
    }
    
    *sum1 = local_sum1;
    *sum2 = local_sum2;
}

/* Alternative function with different recurrence patterns */
static void process_arrays2(int *restrict arr1, int *restrict arr2, 
                           int *restrict acc1, int *restrict acc2,
                           int outer_bound) {
    volatile int bound = outer_bound;
    int tmp1 = *acc1;
    int tmp2 = *acc2;
    
    for (int j = 0; j < bound; ++j) {
        /* Inner loop with multiple independent recurrences */
        for (int i = 0; i < 16; ++i) {  /* Different constant */
            /* Recurrence 1: simple accumulation */
            tmp1 += arr1[i];
            
            /* Recurrence 2: multiplication chain */
            tmp2 *= arr2[i] + 1;
            
            /* Recurrence 3: data-dependent store */
            arr1[i] = arr1[i] + tmp1 - tmp2;
        }
        
        /* Create loop-carried dependency for outer loop */
        volatile int marker = tmp1 ^ tmp2;
        (void)marker;
    }
    
    *acc1 = tmp1;
    *acc2 = tmp2;
}

int main(void) {
    /* Initialize with volatile to prevent constant propagation */
    volatile int outer_iterations = 100;
    
    /* Allocate and initialize arrays */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Seed RNG for initialization */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 256;
        data2[i] = rand() % 256;
    }
    
    /* Accumulators */
    int sum1 = 0;
    int sum2 = 1;  /* Start with 1 for multiplicative identity */
    
    /* Process with first pattern */
    process_arrays(data1, data2, &sum1, &sum2, outer_iterations);
    
    /* Reset and process with second pattern */
    int acc1 = 0;
    int acc2 = 1;
    process_arrays2(data1 + 100, data2 + 100, &acc1, &acc2, outer_iterations / 2);
    
    /* Use results to prevent optimization */
    printf("Results: sum1 = %d, sum2 = %d, acc1 = %d, acc2 = %d\n", 
           sum1, sum2, acc1, acc2);
    
    /* Final volatile use */
    volatile int final_check = sum1 + sum2 + acc1 + acc2;
    (void)final_check;
    
    free(data1);
    free(data2);
    
    return 0;
}

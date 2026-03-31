/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function containing the modulo-schedulable loop */
static void process_arrays(int *restrict data1, int *restrict data2, 
                           int *restrict sum1, int *restrict sum2, 
                           int outer_iter) {
    volatile int outer_counter = outer_iter; /* Prevent optimization */
    int local_sum1 = *sum1;
    int local_sum2 = *sum2;
    
    /* Outer loop - prevents full unrolling of inner loop */
    for (int j = 0; j < outer_counter; ++j) {
        /* Inner loop with constant bound - candidate for modulo scheduling */
        /* Multiple independent recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: mixed operations to create different edges */
            local_sum2 = data2[i] * 3 - local_sum2;
            
            /* Third recurrence: more complex dependency chain */
            data1[i] = data1[i] + (local_sum1 >> 2);
        }
        
        /* Prevent dead code elimination */
        volatile int side_effect = local_sum1 + local_sum2;
        (void)side_effect;
        
        /* Shift arrays to create new patterns */
        for (int i = 0; i < INNER_ITER - 1; ++i) {
            data1[i] = data1[i + 1];
            data2[i] = data2[i + 1];
        }
    }
    
    *sum1 = local_sum1;
    *sum2 = local_sum2;
}

/* Alternative implementation with different recurrence patterns */
static void process_arrays2(int *restrict arr1, int *restrict arr2,
                           int *restrict acc1, int *restrict acc2,
                           int outer_bound) {
    volatile int ob = outer_bound;
    int a1 = *acc1;
    int a2 = *acc2;
    
    for (int j = 0; j < ob; ++j) {
        /* Inner loop with multiple distance-1 dependencies */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* Recurrence 1: accumulation with multiplication */
            a1 = arr1[i] * a1 + 1;
            
            /* Recurrence 2: alternating pattern */
            a2 = (arr2[i] - a2) & 0xFF;
            
            /* Recurrence 3: simple addition chain */
            arr1[i] = arr1[i] + a1;
            
            /* Recurrence 4: subtraction chain */
            arr2[i] = arr2[i] - a2 + i;
        }
        
        volatile int dummy = a1 ^ a2;
        (void)dummy;
    }
    
    *acc1 = a1;
    *acc2 = a2;
}

int main(void) {
    /* Initialize with volatile to prevent constant propagation */
    volatile int outer_bound = 100;
    int sum1 = 0, sum2 = 0;
    int acc1 = 1, acc2 = 0;
    
    /* Allocate and initialize arrays */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    srand(time(NULL));
    
    /* Fill arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 256;
        data2[i] = rand() % 256;
    }
    
    /* Call functions with modulo-schedulable loops */
    process_arrays(data1, data2, &sum1, &sum2, outer_bound);
    process_arrays2(data1 + 64, data2 + 64, &acc1, &acc2, outer_bound / 2);
    
    /* Use results to prevent optimization */
    printf("Results: sum1=%d, sum2=%d, acc1=%d, acc2=%d\n", 
           sum1, sum2, acc1, acc2);
    
    free(data1);
    free(data2);
    
    return 0;
}

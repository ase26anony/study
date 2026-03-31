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
static void process_arrays(int *restrict data1, int *restrict data2, 
                           int *restrict sum1, int *restrict sum2, 
                           int outer_bound) {
    volatile int side_effect = 0;
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - candidate for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum1 depends on previous sum1 value */
            *sum1 = data1[i] + *sum1;
            
            /* Second recurrence: sum2 depends on previous sum2 value */
            *sum2 = data2[i] + *sum2;
            
            /* Third recurrence with different operation to create varied edges */
            data1[i] = data1[i] * 3 - *sum1;
        }
        
        /* Prevent dead code elimination of outer loop */
        side_effect = *sum1 + *sum2;
        
        /* Small operation to vary data between outer iterations */
        if (j % 2 == 0) {
            data1[0] += 1;
        }
    }
}

/* Alternative function with more complex recurrence patterns */
static void complex_recurrence(int *restrict arr1, int *restrict arr2,
                               int *restrict acc1, int *restrict acc2,
                               int outer_bound) {
    volatile int marker = 0;
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Multiple independent recurrences in the inner loop */
        for (int i = 1; i < INNER_ITER; ++i) {
            /* Loop-carried dependency: current depends on previous iteration */
            arr1[i] = arr1[i-1] + arr2[i];
            
            /* Another recurrence with multiplication */
            *acc1 = *acc1 * 2 + arr1[i];
            
            /* Third recurrence with subtraction */
            *acc2 = *acc2 - arr2[i] + i;
            
            /* Additional operation creating more edges */
            arr2[i] = arr2[i] + (arr1[i] >> 1);
        }
        
        marker = *acc1 ^ *acc2;
        
        /* Reset part of data for next outer iteration */
        if (j < outer_bound - 1) {
            arr1[0] = rand() % 100;
        }
    }
}

int main(void) {
    /* Use volatile to prevent constant propagation */
    volatile int outer_bound = 100;
    int actual_bound = outer_bound;
    
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
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
    }
    
    /* Accumulators for recurrences */
    int sum1 = 0;
    int sum2 = 0;
    
    /* First test case: simple recurrences */
    process_arrays(data1, data2, &sum1, &sum2, actual_bound);
    
    printf("After process_arrays: sum1 = %d, sum2 = %d\n", sum1, sum2);
    
    /* Reset for second test case */
    int acc1 = 1;
    int acc2 = 1000;
    
    /* Second test case: more complex recurrences */
    complex_recurrence(data1, data2, &acc1, &acc2, actual_bound / 2);
    
    printf("After complex_recurrence: acc1 = %d, acc2 = %d\n", acc1, acc2);
    
    /* Use results to prevent optimization */
    int final_result = sum1 + sum2 + acc1 + acc2;
    printf("Final result: %d\n", final_result);
    
    free(data1);
    free(data2);
    
    return final_result != 0 ? 0 : 1;
}

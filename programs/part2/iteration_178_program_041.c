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
                          volatile int outer_bound) {
    volatile int side_effect = 0;
    
    /* Outer loop to provide multiple contexts */
    for (int j = 0; j < outer_bound; ++j) {
        /* Reset accumulators each outer iteration */
        int local_sum1 = 0;
        int local_sum2 = 0;
        
        /* INNER LOOP - Target for modulo scheduling */
        /* Multiple independent recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            local_sum1 = data1[i] + local_sum1;  /* distance=1 dependency */
            
            /* Second recurrence: mixed operations */
            local_sum2 = data2[i] * 3 - local_sum2;  /* another distance=1 dependency */
            
            /* Third recurrence: more complex operation */
            data1[i] = local_sum1 * 2 + data1[i];  /* self-dependency through array */
        }
        
        /* Prevent dead code elimination */
        side_effect = local_sum1 + local_sum2;
        
        /* Accumulate to outer sums */
        *sum1 += local_sum1;
        *sum2 += local_sum2;
    }
    
    /* Use side effect to prevent optimization */
    if (side_effect == 0) {
        printf("Never happens\n");
    }
}

/* Alternative loop structure with array recurrence */
static void array_recurrence(int *restrict arr1, int *restrict arr2, 
                            volatile int outer_bound) {
    /* Outer loop */
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with array-carried dependency */
        for (int i = 1; i < INNER_ITER; ++i) {
            /* True loop-carried dependency through array */
            arr1[i] = arr1[i-1] * 2 + arr2[i];  /* distance=1 */
            
            /* Independent recurrence */
            arr2[i] = arr2[i] + arr1[i] / 3;    /* distance=0 or 1 depending on analysis */
        }
        
        /* Prevent optimization */
        arr1[0] = arr2[INNER_ITER-1];
    }
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
    
    /* Seed RNG for initialization */
    srand(time(NULL));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
    }
    
    /* Accumulators */
    int sum1 = 0;
    int sum2 = 0;
    
    /* Process with first loop structure */
    process_arrays(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Also test array recurrence pattern */
    array_recurrence(data1, data2, outer_bound / 2);
    
    /* Use results to prevent optimization */
    printf("Results: sum1 = %d, sum2 = %d\n", sum1, sum2);
    printf("Array samples: %d, %d\n", data1[0], data2[0]);
    
    free(data1);
    free(data2);
    
    return 0;
}

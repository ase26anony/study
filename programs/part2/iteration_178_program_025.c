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
void process_arrays(int *restrict data1, int *restrict data2, 
                    int *restrict sum1, int *restrict sum2, 
                    int outer_bound) {
    volatile int side_effect = 0;
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - candidate for modulo scheduling */
        int local_sum1 = *sum1;
        int local_sum2 = *sum2;
        
        /* This inner loop has:
         * 1. Constant iteration count (32)
         * 2. Multiple recurrences (sum1, sum2)
         * 3. Loop-carried dependencies
         * 4. Simple integer operations
         */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with loop-carried dependency */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: different operation to create separate edges */
            local_sum2 = data2[i] * local_sum2 + 1;
            
            /* Third recurrence: mixed operations to increase edge variety */
            if (i > 0) {
                data1[i] = data1[i-1] + data2[i];  /* Simple recurrence pattern */
            }
        }
        
        *sum1 = local_sum1;
        *sum2 = local_sum2;
        
        /* Prevent dead code elimination */
        side_effect = j;
    }
    
    /* Use side_effect to prevent optimization */
    if (side_effect > 1000000) {
        printf("Impossible branch\n");
    }
}

/* Alternative implementation with different recurrence patterns */
void process_arrays_variant(int *restrict arr1, int *restrict arr2,
                           int outer_bound) {
    volatile int counter = 0;
    int acc1 = 1, acc2 = 0, acc3 = 100;
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Multiple independent recurrences in the inner loop */
        for (int i = 0; i < 16; ++i) {  /* Different constant for variety */
            /* Chain of dependent operations */
            acc1 = acc1 * arr1[i] + i;
            acc2 = acc2 - arr2[i];
            acc3 = (acc3 + arr1[i]) * 3;
            
            /* Cross-iteration dependency with distance 1 */
            if (i < 15) {
                arr1[i+1] = arr1[i] + acc1;
            }
        }
        
        counter = j;
        
        /* Small operation to break monotony */
        arr2[0] = acc1 + acc2;
    }
    
    /* Ensure results are used */
    arr1[0] = acc1;
    arr2[1] = acc2;
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
        data2[i] = rand() % 100 + 1;  /* Avoid zero for multiplication */
    }
    
    int sum1 = 0, sum2 = 1;  /* Start sum2 at 1 for multiplicative recurrence */
    
    /* Process with first pattern */
    process_arrays(data1, data2, &sum1, &sum2, actual_bound);
    
    /* Process with variant pattern */
    process_arrays_variant(data1, data2, actual_bound / 2);
    
    /* Use results to prevent optimization */
    printf("Results: sum1 = %d, sum2 = %d, data1[0] = %d, data2[1] = %d\n",
           sum1, sum2, data1[0], data2[1]);
    
    free(data1);
    free(data2);
    
    return 0;
}

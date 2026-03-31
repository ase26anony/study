/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function with restrict to avoid aliasing issues */
static void compute_sums(int *restrict data1, int *restrict data2, 
                         int *restrict sum1, int *restrict sum2, 
                         int outer_bound) {
    volatile int side_effect = 0;
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - candidate for modulo scheduling */
        int local_sum1 = *sum1;
        int local_sum2 = *sum2;
        
        /* Multiple independent recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: addition chain */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: mixed operations to create different dependencies */
            local_sum2 = (data2[i] * 3) - local_sum2;
            
            /* Third recurrence: more complex operation */
            data1[i] = data1[i] + (local_sum1 >> 2);
        }
        
        /* Update accumulators */
        *sum1 = local_sum1;
        *sum2 = local_sum2;
        
        /* Side effect to prevent dead code elimination */
        side_effect = j;
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
    int sum1 = 0;
    int sum2 = 0;
    
    /* Call the function containing the target loop */
    compute_sums(data1, data2, &sum1, &sum2, actual_bound);
    
    /* Use results to prevent optimization */
    printf("Final sums: sum1 = %d, sum2 = %d\n", sum1, sum2);
    
    /* Cleanup */
    free(data1);
    free(data2);
    
    return 0;
}

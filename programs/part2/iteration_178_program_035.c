/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function with restrict to guarantee no aliasing */
static void compute_recurrences(int *restrict data1, int *restrict data2, 
                                int *restrict sum1, int *restrict sum2,
                                volatile int outer_bound) {
    volatile int side_effect = 0;
    
    /* Outer loop to provide multiple contexts */
    for (int j = 0; j < outer_bound; ++j) {
        int local_sum1 = *sum1;
        int local_sum2 = *sum2;
        
        /* Inner loop with constant bound - candidate for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with loop-carried dependency */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: different operation to create separate dependency chain */
            local_sum2 = data2[i] * local_sum2 + i;
            
            /* Third recurrence: mixed operations to increase edge complexity */
            data1[i] = data1[i] - local_sum1 / 2;
        }
        
        /* Update global sums */
        *sum1 = local_sum1;
        *sum2 = local_sum2;
        
        /* Side effect to prevent dead code elimination */
        side_effect = j;
    }
    
    /* Use side effect to prevent optimization */
    if (side_effect > 1000) printf("Impossible\n");
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
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = (rand() % 50) + 1;  /* Ensure non-zero for multiplication */
    }
    
    /* Accumulation variables */
    int sum1 = 0;
    int sum2 = 1;  /* Start with 1 for multiplication */
    
    /* Call the function containing the target loops */
    compute_recurrences(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Print results to ensure computation isn't optimized away */
    printf("Final sums: sum1 = %d, sum2 = %d\n", sum1, sum2);
    
    /* Cleanup */
    free(data1);
    free(data2);
    
    return 0;
}

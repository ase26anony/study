/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Force outer loop to not be optimized away */
volatile int outer_bound = 100;

/* Function with restrict to guarantee no aliasing */
void compute_recurrences(int *restrict data1, int *restrict data2, 
                         int *restrict sum1, int *restrict sum2, 
                         int outer_iter) {
    int local_sum1 = *sum1;
    int local_sum2 = *sum2;
    
    /* Outer loop to provide multiple contexts */
    for (int j = 0; j < outer_iter; ++j) {
        /* Inner loop with constant bound - candidate for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with loop-carried dependency */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: different operation to create separate dependency chain */
            local_sum2 = data2[i] * local_sum2 + i;
            
            /* Third recurrence: mixed operations to increase edge complexity */
            data1[i] = data1[i] - local_sum1 * 3;
        }
        
        /* Prevent dead code elimination without inhibiting modulo scheduling */
        volatile int side_effect = local_sum1;
        (void)side_effect;
    }
    
    *sum1 = local_sum1;
    *sum2 = local_sum2;
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100 + 1;  /* Ensure non-zero for multiplication */
    }
    
    /* Accumulation variables */
    int sum1 = 0;
    int sum2 = 1;  /* Start with 1 for multiplication */
    
    /* Call the function containing the modulo-schedulable loop */
    compute_recurrences(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Use results to prevent optimization */
    printf("Final sums: sum1 = %d, sum2 = %d\n", sum1, sum2);
    
    free(data1);
    free(data2);
    
    return 0;
}

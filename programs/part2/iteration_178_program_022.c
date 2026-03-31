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
                         volatile int outer_bound) {
    int local_sum1 = 0;
    int local_sum2 = 0;
    
    /* Outer loop with volatile bound to prevent full optimization */
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - candidate for modulo scheduling */
        /* Multiple independent recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            local_sum1 = data1[i] + local_sum1;  /* distance=1 dependency */
            
            /* Second recurrence: mixed operations */
            local_sum2 = (data2[i] * 3) - local_sum2;  /* different latency pattern */
            
            /* Third recurrence: more complex dependency chain */
            if (i > 0) {
                data1[i] = data1[i-1] + data1[i];  /* explicit distance=1 dependency */
            }
        }
        
        /* Prevent dead code elimination */
        volatile int sink __attribute__((unused)) = local_sum1 + local_sum2;
    }
    
    *sum1 = local_sum1;
    *sum2 = local_sum2;
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
        data2[i] = rand() % 100;
    }
    
    int sum1, sum2;
    
    /* Call the function containing the target loops */
    compute_sums(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Use results to prevent optimization */
    printf("Final sums: %d, %d\n", sum1, sum2);
    
    free(data1);
    free(data2);
    
    return 0;
}

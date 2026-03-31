/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Use restrict to guarantee no aliasing for better dependence analysis */
static void compute_sums(int *restrict data1, int *restrict data2, 
                         int *restrict sum1, int *restrict sum2, 
                         volatile int outer_bound) {
    int local_sum1 = 0;
    int local_sum2 = 0;
    
    /* Outer loop with volatile bound to prevent full unrolling */
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - prime candidate for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with loop-carried dependency */
            local_sum1 = data1[i] + local_sum1;  /* distance=1 edge */
            
            /* Second recurrence: different operation to create separate dependency chain */
            local_sum2 = data2[i] * 3 + local_sum2;  /* Another distance=1 edge */
            
            /* Third recurrence: mixed operations to increase edge variety */
            local_sum1 = local_sum1 - (data1[i] >> 2);  /* Additional distance=1 edge */
        }
        
        /* Prevent dead code elimination without inhibiting modulo scheduling */
        volatile int dummy = local_sum1;
        (void)dummy;
        
        /* Small side effect to prevent outer loop removal */
        data1[0] = j % 256;
    }
    
    *sum1 = local_sum1;
    *sum2 = local_sum2;
}

int main(void) {
    /* Initialize with volatile to prevent constant propagation */
    volatile int outer_bound = 100;
    int data1[ARRAY_SIZE];
    int data2[ARRAY_SIZE];
    int sum1 = 0, sum2 = 0;
    
    /* Seed RNG for array initialization */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
    }
    
    /* Call the function containing the modulo-schedulable loop */
    compute_sums(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Use results to prevent optimization */
    printf("Final sums: %d, %d\n", sum1, sum2);
    
    return 0;
}

/* modulo-sched-test.c
 * Test program to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_LOOP_BOUND 32  /* Small constant bound suitable for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function containing the target loop structure */
void compute_sums(int *restrict data1, int *restrict data2, 
                  int *restrict sum1, int *restrict sum2, 
                  volatile int outer_bound) {
    int local_sum1 = 0;
    int local_sum2 = 0;
    
    /* Outer loop with runtime-dependent bound */
    for (int j = 0; j < outer_bound; ++j) {
        /* Target inner loop with constant bound and recurrences */
        for (int i = 0; i < INNER_LOOP_BOUND; ++i) {
            /* First recurrence: sum reduction with loop-carried dependency */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: different operation to create separate dependency chain */
            local_sum2 = data2[i] * 3 - local_sum2;
            
            /* Third recurrence: mixed operations to increase edge complexity */
            data1[i] = data1[i] + local_sum1 * 2;
        }
        
        /* Prevent dead code elimination without inhibiting modulo scheduling */
        volatile int dummy = local_sum1;
        (void)dummy;
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
        data2[i] = rand() % 100;
    }
    
    /* Use volatile to prevent constant propagation */
    volatile int outer_bound = 100;
    int sum1, sum2;
    
    /* Call the function containing the modulo-schedulable loop */
    compute_sums(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Use results to prevent optimization */
    printf("Result: sum1 = %d, sum2 = %d\n", sum1, sum2);
    
    free(data1);
    free(data2);
    
    return 0;
}

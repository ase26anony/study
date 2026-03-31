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
void compute_sums(int *restrict data1, int *restrict data2, 
                  int *restrict sum1, int *restrict sum2, 
                  int outer_bound) {
    int local_sum1 = 0;
    int local_sum2 = 0;
    
    /* Outer loop with runtime bound */
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - target for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with addition */
            local_sum1 += data1[(j * INNER_ITER + i) % ARRAY_SIZE];
            
            /* Second recurrence: different operation (multiplication) */
            local_sum2 = local_sum2 * 2 + data2[(j * INNER_ITER + i) % ARRAY_SIZE];
            
            /* Third recurrence: mixed operations to create more edges */
            data1[(j * INNER_ITER + i + 1) % ARRAY_SIZE] = 
                data1[(j * INNER_ITER + i) % ARRAY_SIZE] + 3;
        }
        
        /* Prevent dead code elimination without inhibiting modulo scheduling */
        volatile int prevent_elim = local_sum1;
        (void)prevent_elim;
    }
    
    *sum1 = local_sum1;
    *sum2 = local_sum2;
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
    }
    
    /* Volatile to prevent constant propagation */
    volatile int outer_bound = 100;
    int sum1, sum2;
    
    /* Call the function containing the target loops */
    compute_sums(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Use results to prevent optimization */
    printf("Result: sum1 = %d, sum2 = %d\n", sum1, sum2);
    
    free(data1);
    free(data2);
    
    return 0;
}

/* modulo-sched-coverage.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-coverage.c -o modulo-sched-coverage
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
        volatile int prevent_elim = j;  /* Side effect to prevent elimination */
        
        /* Inner loop - target for modulo scheduling */
        /* Multiple independent recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            local_sum1 += data1[(j * INNER_ITER + i) % ARRAY_SIZE];
            
            /* Second recurrence: mixed operations with dependency chain */
            local_sum2 = data2[(j * INNER_ITER + i) % ARRAY_SIZE] - local_sum2 * 3;
            
            /* Third recurrence: different pattern to create more edges */
            data1[(j * INNER_ITER + (i + 1)) % ARRAY_SIZE] = 
                data1[(j * INNER_ITER + i) % ARRAY_SIZE] + 7;
        }
        
        /* Use the volatile to prevent dead code elimination */
        if (prevent_elim > outer_bound) { /* Always false, but compiler doesn't know */
            printf("Never happens\n");
        }
    }
    
    *sum1 = local_sum1;
    *sum2 = local_sum2;
}

int main(void) {
    /* Initialize data arrays */
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
    
    /* Volatile outer bound to prevent constant propagation */
    volatile int outer_bound = 100;
    int actual_bound = outer_bound;
    
    int sum1, sum2;
    
    /* Call the computation function */
    compute_sums(data1, data2, &sum1, &sum2, actual_bound);
    
    /* Use results to prevent optimization */
    printf("Final sums: %d, %d\n", sum1, sum2);
    
    free(data1);
    free(data2);
    
    return 0;
}

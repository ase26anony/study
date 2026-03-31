/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler
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
                          int outer_bound) {
    volatile int side_effect = 0;
    
    /* Outer loop to provide context */
    for (int j = 0; j < outer_bound; ++j) {
        int local_sum1 = *sum1;
        int local_sum2 = *sum2;
        
        /* Inner loop - target for modulo scheduling */
        /* Multiple recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            local_sum1 += data1[i + j % (ARRAY_SIZE - INNER_ITER)];
            
            /* Second recurrence: mixed operations with dependency */
            local_sum2 = data2[i + j % (ARRAY_SIZE - INNER_ITER)] * 3 - local_sum2;
            
            /* Third recurrence: accumulation with multiplication */
            /* Creates another dependency chain */
            if (i > 0) {
                local_sum1 = local_sum1 * 2 - data1[i-1 + j % (ARRAY_SIZE - INNER_ITER)];
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

int main(void) {
    /* Use volatile to prevent constant propagation */
    volatile int outer_bound = 100;
    int bound = outer_bound;
    
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
    
    /* Accumulation variables */
    int sum1 = 0;
    int sum2 = 0;
    
    /* Process the arrays */
    process_arrays(data1, data2, &sum1, &sum2, bound);
    
    /* Use results to prevent optimization */
    printf("Results: sum1 = %d, sum2 = %d\n", sum1, sum2);
    
    free(data1);
    free(data2);
    
    return 0;
}

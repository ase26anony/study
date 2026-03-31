/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function with restrict qualifiers to avoid aliasing issues */
static void compute_sums(int *restrict data1, int *restrict data2, 
                         int *restrict sum1, int *restrict sum2, 
                         int outer_bound) {
    volatile int outer_volatile = outer_bound; /* Prevent optimization */
    int local_sum1 = 0, local_sum2 = 0;
    
    /* Outer loop with volatile bound to prevent unrolling */
    for (int j = 0; j < outer_volatile; ++j) {
        /* Inner loop with constant bound - target for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with addition */
            local_sum1 += data1[i + j];  /* distance=1 dependency */
            
            /* Second recurrence: different operation (multiplication) */
            local_sum2 = local_sum2 * 2 + data2[i + j];  /* Another distance=1 dependency */
            
            /* Third independent recurrence for more edges */
            data1[i + j] = data1[i + j] + (local_sum1 & 0xFF); /* Mixed operation */
        }
        
        /* Prevent dead code elimination */
        volatile int side_effect = local_sum1;
        (void)side_effect;
    }
    
    *sum1 = local_sum1;
    *sum2 = local_sum2;
}

int main(void) {
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
    
    int sum1 = 0, sum2 = 0;
    volatile int outer_bound = 100; /* Runtime value to prevent constant propagation */
    
    /* Call the function containing the target loops */
    compute_sums(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Use results to prevent optimization */
    printf("Final sums: %d, %d\n", sum1, sum2);
    
    free(data1);
    free(data2);
    
    return 0;
}

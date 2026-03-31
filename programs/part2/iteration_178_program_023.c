/* modulo-sched-coverage.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function containing the modulo-schedulable loop */
static void process_arrays(int *restrict data1, int *restrict data2, 
                          int *restrict sum1, int *restrict sum2, 
                          int outer_iter) {
    volatile int outer_counter = outer_iter; /* Prevent optimization */
    int local_sum1 = *sum1;
    int local_sum2 = *sum2;
    
    /* Outer loop - prevents full unrolling of inner loop */
    for (int j = 0; j < outer_counter; ++j) {
        /* Inner loop with constant bound - prime candidate for modulo scheduling */
        /* Multiple recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with addition */
            local_sum1 += data1[i + j];  /* distance=1 dependency on local_sum1 */
            
            /* Second recurrence: different operation (multiplication) */
            local_sum2 = local_sum2 * 3 + data2[i + j];  /* distance=1 dependency on local_sum2 */
            
            /* Third recurrence: mixed operations to create more edges */
            data1[i + j] = data1[i + j] - local_sum1 / 2;  /* anti-dependency on data1 */
        }
        
        /* Prevent dead code elimination */
        volatile int side_effect = local_sum1 + local_sum2;
        (void)side_effect;
    }
    
    *sum1 = local_sum1;
    *sum2 = local_sum2;
}

/* Alternative version with more complex recurrences */
static void complex_recurrence(int *restrict arr1, int *restrict arr2, 
                              int *restrict acc1, int *restrict acc2,
                              int outer_bound) {
    int t1 = *acc1;
    int t2 = *acc2;
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with multiple interleaved recurrences */
        for (int i = 0; i < 16; ++i) {  /* Different small constant */
            /* Chain 1: Linear recurrence */
            t1 = arr1[i] + t1 * 2 - 1;
            
            /* Chain 2: Different recurrence pattern */
            t2 = t2 + arr2[i] * t1;
            
            /* Chain 3: Independent recurrence */
            arr1[i] = arr1[i] + i;
            
            /* Chain 4: Another recurrence with subtraction */
            arr2[i] = arr2[i] - t2 / 4;
        }
        
        /* Small computation between inner loops */
        volatile int temp = t1 ^ t2;
        (void)temp;
    }
    
    *acc1 = t1;
    *acc2 = t2;
}

int main(void) {
    /* Initialize with volatile to prevent constant propagation */
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
    
    /* Accumulators for recurrences */
    int sum1 = 0;
    int sum2 = 1;  /* Start with 1 for multiplicative recurrence */
    
    /* Process with modulo-schedulable loops */
    process_arrays(data1, data2, &sum1, &sum2, actual_bound);
    
    /* Second test case with different pattern */
    int acc1 = 5;
    int acc2 = 7;
    complex_recurrence(data1, data2, &acc1, &acc2, actual_bound / 2);
    
    /* Use results to prevent optimization */
    printf("Results: sum1=%d, sum2=%d, acc1=%d, acc2=%d\n", 
           sum1, sum2, acc1, acc2);
    
    /* Verify no array overflow */
    volatile int check = data1[0] + data2[0];
    (void)check;
    
    free(data1);
    free(data2);
    
    return 0;
}

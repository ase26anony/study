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
static void process_arrays(int *restrict data1, int *restrict data2, 
                          int *restrict sum1, int *restrict sum2, 
                          int outer_bound) {
    volatile int side_effect = 0;
    
    /* Outer loop to provide multiple contexts */
    for (int j = 0; j < outer_bound; ++j) {
        int local_sum1 = *sum1;
        int local_sum2 = *sum2;
        
        /* Inner loop - target for modulo scheduling */
        /* Multiple recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            local_sum1 += data1[i + j % (ARRAY_SIZE - INNER_ITER)];
            
            /* Second recurrence: mixed operations with multiplication */
            local_sum2 = local_sum2 * 3 + data2[i + j % (ARRAY_SIZE - INNER_ITER)];
            
            /* Third recurrence: subtraction chain */
            if (i > 0) {
                data1[i + j % (ARRAY_SIZE - INNER_ITER)] -= 
                    data1[i - 1 + j % (ARRAY_SIZE - INNER_ITER)];
            }
        }
        
        *sum1 = local_sum1;
        *sum2 = local_sum2;
        
        /* Prevent dead code elimination */
        side_effect = j;
    }
    
    /* Use side_effect to prevent optimization */
    if (side_effect < 0) {
        printf("Impossible branch\n");
    }
}

/* Alternative loop structure with different dependency patterns */
static void process_arrays_variant(int *restrict arr1, int *restrict arr2,
                                  int *restrict acc1, int *restrict acc2,
                                  int outer_bound) {
    volatile int dummy = 0;
    
    for (int j = 0; j < outer_bound; ++j) {
        int t1 = *acc1;
        int t2 = *acc2;
        int t3 = 1;
        
        /* Loop with multiple interleaved recurrences */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* Chain 1: Linear recurrence */
            t1 = arr1[i] + t1 * 2;
            
            /* Chain 2: Different latency pattern */
            t2 = t2 - arr2[i] + 5;
            
            /* Chain 3: Geometric progression */
            t3 = t3 * 7 + i;
            
            /* Cross-iteration store with distance 1 */
            if (i < INNER_ITER - 1) {
                arr1[i + 1] = arr1[i] + t2;
            }
        }
        
        *acc1 = t1;
        *acc2 = t2;
        dummy += t3;
    }
    
    if (dummy == 0) {
        printf("Never happens\n");
    }
}

int main(void) {
    /* Use volatile to prevent constant propagation */
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
    
    int sum1 = 0;
    int sum2 = 0;
    int acc1 = 0;
    int acc2 = 0;
    
    /* Call both variants to increase scheduling opportunities */
    process_arrays(data1, data2, &sum1, &sum2, actual_bound);
    process_arrays_variant(data1, data2, &acc1, &acc2, actual_bound / 2);
    
    /* Use results to prevent optimization */
    printf("Results: sum1=%d, sum2=%d, acc1=%d, acc2=%d\n", 
           sum1, sum2, acc1, acc2);
    
    free(data1);
    free(data2);
    
    return 0;
}

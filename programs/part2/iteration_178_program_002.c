/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function with loop structure suitable for modulo scheduling */
static void process_loops(int *restrict data1, int *restrict data2, 
                          int *restrict sum1, int *restrict sum2, 
                          int outer_bound) {
    volatile int side_effect = 0;  /* Prevent outer loop elimination */
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - candidate for modulo scheduling */
        int local_sum1 = *sum1;
        int local_sum2 = *sum2;
        
        /* Multiple recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: addition with loop-carried dependency */
            local_sum1 = data1[i] + local_sum1;  /* sum1 += data1[i] */
            
            /* Second recurrence: mixed operations to create different edges */
            local_sum2 = data2[i] * 3 - local_sum2;  /* Different latency pattern */
            
            /* Third independent recurrence with multiplication */
            /* Creates additional dependency edges for logging */
            if (i > 0) {
                data1[i] = data1[i-1] * 2 + data1[i];  /* Simple recurrence pattern */
            }
        }
        
        *sum1 = local_sum1;
        *sum2 = local_sum2;
        
        /* Prevent dead code elimination of outer loop */
        side_effect = j;
    }
}

/* Alternative implementation with more complex recurrence patterns */
static void complex_recurrence(int *restrict arr1, int *restrict arr2, 
                               int outer_bound) {
    volatile int counter = 0;
    
    for (int k = 0; k < outer_bound; ++k) {
        int acc1 = arr1[0];
        int acc2 = arr2[0];
        int acc3 = 0;
        
        /* Inner loop with multiple interleaved recurrences */
        for (int i = 1; i < INNER_ITER; ++i) {
            /* Chain 1: Simple additive recurrence */
            acc1 = arr1[i] + acc1;
            
            /* Chain 2: Multiplicative recurrence */
            acc2 = arr2[i] * acc2 - i;
            
            /* Chain 3: Mixed recurrence with dependency on previous iteration */
            acc3 = (acc1 + acc2) * 2 - acc3;
            
            /* Additional array recurrence to create more edges */
            arr1[i] = arr1[i-1] + arr2[i];
        }
        
        /* Use results to prevent optimization */
        arr1[0] = acc1;
        arr2[0] = acc2;
        counter = k;
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
    
    /* Process loops - this should trigger modulo scheduling */
    process_loops(data1, data2, &sum1, &sum2, actual_bound);
    
    /* Also test the complex recurrence pattern */
    complex_recurrence(data1, data2, actual_bound / 2);
    
    /* Print results to prevent dead code elimination */
    printf("Results: sum1 = %d, sum2 = %d\n", sum1, sum2);
    printf("First few array values: %d, %d, %d\n", data1[0], data1[1], data2[0]);
    
    free(data1);
    free(data2);
    
    return 0;
}

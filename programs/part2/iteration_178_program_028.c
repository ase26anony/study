/* modulo-sched-coverage.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_LOOP_BOUND 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Use restrict to guarantee no aliasing for better dependence analysis */
static void process_loops(int *restrict data1, int *restrict data2, 
                          int *restrict sum1, int *restrict sum2, 
                          volatile int outer_bound) {
    volatile int side_effect = 0;  /* Prevent outer loop elimination */
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Reset accumulators each outer iteration */
        int local_sum1 = 0;
        int local_sum2 = 0;
        
        /* Target inner loop with multiple recurrences - candidate for modulo scheduling */
        for (int i = 0; i < INNER_LOOP_BOUND; ++i) {
            /* First recurrence: simple accumulation (distance=1 dependency) */
            local_sum1 = data1[i] + local_sum1;  /* sum1 depends on previous sum1 */
            
            /* Second recurrence: different operation with different latency characteristics */
            local_sum2 = data2[i] * 3 + local_sum2;  /* Multiply-add chain */
            
            /* Third recurrence: mixed operations to create more edges */
            if (i > 0) {
                /* Cross-iteration dependency with distance=1 */
                data1[i] = data1[i-1] + data2[i];  /* Another recurrence */
            }
        }
        
        /* Update global sums */
        *sum1 += local_sum1;
        *sum2 += local_sum2;
        
        /* Prevent dead code elimination of outer loop */
        side_effect = j;
    }
    
    /* Use side_effect to prevent optimization */
    if (side_effect < 0) {
        printf("Impossible branch\n");
    }
}

/* Alternative version with more complex recurrences */
static void process_loops_complex(int *restrict arr1, int *restrict arr2,
                                  int *restrict sum1, int *restrict sum2,
                                  volatile int outer_bound) {
    volatile int dummy = 0;
    
    for (int iter = 0; iter < outer_bound; ++iter) {
        int acc1 = 0;
        int acc2 = 1;  /* Start with 1 for multiplicative chain */
        int acc3 = arr1[0];
        
        /* Inner loop designed specifically for modulo scheduling */
        for (int i = 0; i < INNER_LOOP_BOUND; ++i) {
            /* Chain 1: Simple additive reduction */
            acc1 = arr1[i] - acc1;  /* Subtraction creates different latency pattern */
            
            /* Chain 2: Multiplicative reduction */
            acc2 = arr2[i] * acc2;
            
            /* Chain 3: Recurrence with array update */
            if (i < INNER_LOOP_BOUND - 1) {
                arr1[i+1] = arr1[i] * 2 + acc3;
            }
            
            /* Chain 4: Mixed operation with constant */
            acc3 = (acc3 << 1) | 1;
        }
        
        *sum1 ^= acc1;  /* Use XOR to prevent algebraic simplification */
        *sum2 += acc2;
        dummy = iter;
    }
    
    if (dummy < 0) {
        *sum1 = 0;
    }
}

int main(void) {
    /* Initialize with volatile source to prevent constant propagation */
    volatile int outer_iterations = 100;
    const int actual_outer = outer_iterations;
    
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
        data1[i] = rand() % 256;
        data2[i] = rand() % 256;
    }
    
    int total_sum1 = 0;
    int total_sum2 = 0;
    
    /* Call the loop processing function */
    process_loops(data1, data2, &total_sum1, &total_sum2, actual_outer);
    
    /* Also call complex version to increase scheduling opportunities */
    int total_sum3 = 0;
    int total_sum4 = 0;
    process_loops_complex(data1, data2, &total_sum3, &total_sum4, actual_outer / 2);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d, %d, %d, %d\n", total_sum1, total_sum2, total_sum3, total_sum4);
    
    free(data1);
    free(data2);
    
    return 0;
}

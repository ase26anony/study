/* modulo-sched-coverage.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-coverage.c -o modulo-sched-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Force outer loop to not be optimized away */
volatile int outer_bound = 100;

/* Initialize arrays with pseudo-random data */
void init_arrays(int *restrict arr1, int *restrict arr2, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
    }
}

/* Main function with nested loops containing multiple recurrences */
int main(void) {
    int data1[ARRAY_SIZE];
    int data2[ARRAY_SIZE];
    volatile int side_effect = 0;  /* Prevent dead code elimination */
    
    /* Initialize with random data */
    srand(time(NULL));
    init_arrays(data1, data2, ARRAY_SIZE);
    
    int total_sum1 = 0;
    int total_sum2 = 0;
    int total_prod = 0;
    
    /* Outer loop - prevents full unrolling of inner loop */
    for (int outer = 0; outer < outer_bound; outer++) {
        int sum1 = 0;
        int sum2 = 0;
        int prod = 1;  /* Multiplicative recurrence */
        
        /* INNER LOOP - Target for modulo scheduling */
        /* This loop has:
         * 1. Constant small iteration count (32)
         * 2. Multiple recurrences with loop-carried dependencies
         * 3. Mixed operations (add, multiply)
         * 4. Array accesses with stride 1
         */
        for (int i = 0; i < INNER_ITER; i++) {
            /* Recurrence 1: Sum reduction - distance=1 dependency */
            sum1 = data1[i] + sum1;
            
            /* Recurrence 2: Another sum reduction - independent chain */
            sum2 = data2[i] + sum2;
            
            /* Recurrence 3: Multiplicative recurrence - different latency */
            prod = data1[i] * prod;
            
            /* Additional operation: mixed arithmetic to create more edges */
            if (i > 0) {
                /* Creates distance=1 dependency on previous iteration */
                data1[i] = data1[i-1] + data2[i];
            }
        }
        
        /* Accumulate results from inner loop */
        total_sum1 += sum1;
        total_sum2 += sum2;
        total_prod += prod;
        
        /* Side effect to prevent outer loop elimination */
        side_effect = outer;
    }
    
    /* Use results to prevent optimization */
    printf("Results: sum1=%d, sum2=%d, prod=%d, side=%d\n", 
           total_sum1, total_sum2, total_prod, side_effect);
    
    return 0;
}

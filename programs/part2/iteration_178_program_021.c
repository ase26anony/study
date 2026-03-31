/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler
 * Specifically targets the print_node_edges function in modulo-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32      /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Use restrict to prevent aliasing analysis issues */
static void process_loops(int *restrict data1, int *restrict data2, 
                          int *restrict sum1, int *restrict sum2, 
                          volatile int outer_bound) {
    int i, j;
    
    /* Outer loop with volatile bound to prevent unrolling */
    for (j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - candidate for modulo scheduling */
        /* Multiple independent recurrences with different operations */
        for (i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            *sum1 = data1[i] + *sum1;
            
            /* Second recurrence: multiplication-accumulation with different latency */
            *sum2 = data2[i] * 3 + *sum2;
            
            /* Third recurrence: mixed operations to create more edges */
            data1[i] = data1[i] - *sum1 / 2;
        }
        
        /* Prevent dead code elimination without inhibiting modulo scheduling */
        volatile int side_effect = *sum1 + *sum2;
        (void)side_effect;  /* Use the value to prevent optimization */
        
        /* Small data shift to create variation between outer iterations */
        int temp = data1[0];
        for (i = 0; i < INNER_ITER - 1; ++i) {
            data1[i] = data1[i + 1];
        }
        data1[INNER_ITER - 1] = temp;
    }
}

/* Alternative implementation with nested loops and array recurrences */
static void alternative_implementation(int *restrict arr1, int *restrict arr2, 
                                      volatile int iterations) {
    int i, j;
    
    for (j = 0; j < iterations; ++j) {
        /* Multiple recurrence patterns in one loop */
        for (i = 1; i < INNER_ITER; ++i) {
            /* Loop-carried dependency: arr1[i] depends on arr1[i-1] */
            arr1[i] = arr1[i-1] + arr2[i];
            
            /* Independent recurrence with different operation */
            arr2[i] = arr2[i-1] * 2 - arr1[i];
            
            /* Another independent operation to increase edge count */
            arr1[i] = arr1[i] ^ (arr2[i] & 0xFF);
        }
        
        /* Anti-optimization barrier */
        asm volatile("" : : "r"(arr1), "r"(arr2) : "memory");
    }
}

int main(void) {
    int *data1, *data2;
    int sum1 = 0, sum2 = 0;
    volatile int outer_bound = 100;  /* Prevent constant propagation */
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
    data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
    }
    
    /* First test case: reduction patterns */
    printf("Starting modulo scheduling test case 1...\n");
    process_loops(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Second test case: array recurrence patterns */
    printf("Starting modulo scheduling test case 2...\n");
    alternative_implementation(data1, data2, outer_bound / 2);
    
    /* Use results to prevent optimization */
    printf("Results: sum1 = %d, sum2 = %d\n", sum1, sum2);
    printf("Final values: data1[0] = %d, data2[0] = %d\n", data1[0], data2[0]);
    
    free(data1);
    free(data2);
    
    return 0;
}

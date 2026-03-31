/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler
 * Specifically targets the print_node_edges function in modulo-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Use restrict to avoid aliasing issues */
static void process_loops(int *restrict data1, int *restrict data2, 
                          int outer_bound, int *restrict sum1, int *restrict sum2) {
    volatile int side_effect = 0;  /* Prevent dead code elimination */
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - candidate for modulo scheduling */
        /* Multiple recurrences with different operations */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: simple addition reduction */
            *sum1 = data1[i] + *sum1;
            
            /* Second recurrence: mixed operations with dependency chain */
            *sum2 = (data2[i] * 3) - *sum2;
            
            /* Third independent recurrence for more edges */
            data1[i] = data1[i] + 1;  /* Self-update creates another dependency */
        }
        
        /* Side effect to prevent outer loop elimination */
        side_effect = *sum1 + *sum2;
        
        /* Small variation to prevent complete optimization */
        data1[0] = j % 10;
        data2[0] = j % 7;
    }
}

int main(void) {
    /* Volatile to prevent constant propagation */
    volatile int outer_bound = 100;
    
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
    
    /* Process the loops */
    process_loops(data1, data2, outer_bound, &sum1, &sum2);
    
    /* Use results to prevent optimization */
    printf("Final sums: sum1 = %d, sum2 = %d\n", sum1, sum2);
    printf("Sample data: data1[0] = %d, data2[0] = %d\n", data1[0], data2[0]);
    
    free(data1);
    free(data2);
    
    return 0;
}

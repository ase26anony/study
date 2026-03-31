/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler
 * Specifically targets the print_node_edges function in modulo-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Use restrict to prevent aliasing analysis issues */
static void process_loops(int *restrict data1, int *restrict data2, 
                          int outer_bound, int *restrict sum1, int *restrict sum2) {
    volatile int outer_counter = outer_bound; /* Prevent optimization */
    int j;
    
    for (j = 0; j < outer_counter; ++j) {
        int i;
        /* Inner loop with constant bound - candidate for modulo scheduling */
        for (i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum1 depends on previous sum1 value */
            *sum1 = data1[i] + *sum1;
            
            /* Second recurrence: sum2 depends on previous sum2 value */
            *sum2 = data2[i] + *sum2;
            
            /* Third recurrence with different operation (multiply) */
            /* Creates different latency pattern for dependency edges */
            if (i > 0) {
                data1[i] = data1[i-1] * 3 + data1[i];
            }
        }
        
        /* Side effect to prevent dead code elimination */
        volatile int side_effect = *sum1 + *sum2;
        (void)side_effect; /* Suppress unused warning */
        
        /* Modify data pointers slightly to create varying access patterns */
        data1 += 2;
        data2 += 2;
    }
}

/* Alternative version with more complex recurrences */
static void process_loops_complex(int *restrict arr1, int *restrict arr2,
                                  int outer_bound, int *restrict acc1, 
                                  int *restrict acc2, int *restrict acc3) {
    volatile int outer_lim = outer_bound;
    int j;
    
    for (j = 0; j < outer_lim; ++j) {
        int i;
        /* Multiple independent recurrences to create more dependency edges */
        for (i = 0; i < INNER_ITER; ++i) {
            /* Chain 1: Simple addition reduction */
            *acc1 += arr1[i];
            
            /* Chain 2: Multiplication with accumulation */
            *acc2 = arr2[i] * 7 + *acc2;
            
            /* Chain 3: Mixed operations with carry-over */
            if (i > 0) {
                *acc3 = (arr1[i-1] - arr2[i]) * *acc3 + 5;
            } else {
                *acc3 = arr1[i] * arr2[i] + 5;
            }
            
            /* Chain 4: Array recurrence */
            if (i > 1) {
                arr1[i] = arr1[i-1] + arr1[i-2];
            }
        }
        
        /* Prevent optimization */
        volatile int marker = *acc1 ^ *acc2 ^ *acc3;
        (void)marker;
        
        /* Rotate arrays */
        arr1 = (arr1 == arr1) ? arr1 + 1 : arr1; /* Dummy condition */
        arr2 += 1;
    }
}

int main(void) {
    int *data1, *data2;
    int sum1 = 0, sum2 = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    volatile int outer_bound = 100; /* Runtime value prevents unrolling */
    int i;
    
    /* Seed RNG for reproducible array values */
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
    data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
    }
    
    printf("Starting modulo scheduling test...\n");
    
    /* First test: Simple recurrences */
    process_loops(data1, data2, outer_bound, &sum1, &sum2);
    
    /* Re-initialize for second test */
    for (i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
    }
    
    /* Second test: More complex recurrences */
    process_loops_complex(data1, data2, outer_bound, &acc1, &acc2, &acc3);
    
    /* Use results to prevent optimization */
    printf("Results: sum1=%d, sum2=%d, acc1=%d, acc2=%d, acc3=%d\n",
           sum1, sum2, acc1, acc2, acc3);
    
    free(data1);
    free(data2);
    
    return 0;
}

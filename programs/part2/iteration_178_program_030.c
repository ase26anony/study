/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function with restrict to avoid aliasing issues */
void compute_recurrences(int *restrict data1, int *restrict data2, 
                         int *restrict sum1, int *restrict sum2, 
                         volatile int outer_bound) {
    int local_sum1 = 0;
    int local_sum2 = 0;
    
    /* Outer loop with volatile bound to prevent unrolling */
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop - target for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with different operations */
            local_sum1 = data1[i] + local_sum1;      /* Simple add recurrence */
            
            /* Second recurrence: more complex operation chain */
            local_sum2 = (data2[i] * 3) - local_sum2; /* Multiply and subtract recurrence */
            
            /* Third independent recurrence with different latency pattern */
            /* Using array of size INNER_ITER to avoid bounds issues */
            if (i > 0) {
                /* Create loop-carried dependency with distance 1 */
                data1[i] = data1[i-1] + data2[i];    /* Cross-recurrence */
            }
        }
        
        /* Prevent dead code elimination with volatile side effect */
        volatile int side_effect = local_sum1 + local_sum2;
        (void)side_effect;  /* Suppress unused warning */
        
        /* Mix in some additional operations to create more edges */
        for (int i = 0; i < 4; ++i) {
            local_sum1 += j * i;      /* Outer loop carried dependency */
            local_sum2 -= i * 2;
        }
    }
    
    /* Store results */
    *sum1 = local_sum1;
    *sum2 = local_sum2;
}

/* Alternative version with multiple independent recurrences */
void compute_multiple_chains(int *restrict arr1, int *restrict arr2,
                            int *restrict arr3, volatile int outer_bound) {
    int chain1 = 0, chain2 = 1, chain3 = 2;
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with 4 independent recurrence chains */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* Chain 1: Simple addition */
            chain1 = arr1[i] + chain1;
            
            /* Chain 2: Multiplication with constant */
            chain2 = arr2[i] * chain2;
            
            /* Chain 3: Mixed operations */
            chain3 = (arr3[i] - chain3) * 2;
            
            /* Chain 4: Data-dependent with stride */
            if (i >= 2) {
                arr1[i] = arr1[i-2] + arr2[i-1];  /* Distance 2 dependency */
            }
        }
        
        /* Volatile access to prevent optimization */
        volatile int temp = chain1 + chain2 + chain3;
        (void)temp;
    }
    
    /* Use results */
    arr1[0] = chain1;
    arr2[0] = chain2;
    arr3[0] = chain3;
}

int main(void) {
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
        data3[i] = rand() % 100;
    }
    
    /* Volatile outer bound to prevent constant propagation */
    volatile int outer_bound = 100;
    
    int sum1 = 0, sum2 = 0;
    
    /* Call first computation function */
    compute_recurrences(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Call second computation function */
    compute_multiple_chains(data1, data2, data3, outer_bound / 2);
    
    /* Use results to prevent optimization */
    printf("Results: sum1 = %d, sum2 = %d\n", sum1, sum2);
    printf("Array values: data1[0] = %d, data2[0] = %d, data3[0] = %d\n",
           data1[0], data2[0], data3[0]);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}

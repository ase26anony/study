/* modulo-sched-test.c
 * Designed to trigger modulo scheduling debug output in GCC's RTL scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -funroll-loops=0 modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INNER_ITER 32  /* Small constant for modulo scheduling */
#define ARRAY_SIZE 1024

/* Function with restrict to guarantee no aliasing */
void compute_recurrences(int *restrict data1, int *restrict data2, 
                         int *restrict sum1, int *restrict sum2, 
                         volatile int outer_bound) {
    int local_sum1 = 0;
    int local_sum2 = 0;
    
    /* Outer loop with volatile bound to prevent full optimization */
    for (int j = 0; j < outer_bound; ++j) {
        /* Inner loop with constant bound - target for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with different operations */
            local_sum1 = data1[i] + local_sum1;      /* Simple add recurrence */
            
            /* Second recurrence: more complex operation chain */
            local_sum2 = (data2[i] * 3) - local_sum2; /* Multiply and subtract recurrence */
            
            /* Third independent recurrence for more edges */
            data1[i] = data1[i] + (local_sum1 & 0xFF); /* Modulo operation */
        }
        
        /* Prevent dead code elimination with volatile side effect */
        volatile int side_effect = local_sum1;
        (void)side_effect;  /* Suppress unused warning */
        
        /* Small stride to change access pattern slightly */
        data1 += 1;
        data2 += 1;
    }
    
    *sum1 = local_sum1;
    *sum2 = local_sum2;
}

/* Alternative version with multiple arrays for more complex dependencies */
void compute_multiple_recurrences(int *restrict arr1, int *restrict arr2,
                                  int *restrict arr3, int *restrict arr4,
                                  volatile int outer_bound) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    for (int j = 0; j < outer_bound; ++j) {
        /* Target inner loop for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* Four independent recurrence chains */
            acc1 = arr1[i] + acc1;           /* Simple addition */
            acc2 = arr2[i] * 2 - acc2;       /* Multiply and subtract */
            acc3 = (arr3[i] >> 1) + acc3;    /* Shift and add */
            acc4 = arr4[i] ^ acc4;           /* XOR recurrence */
            
            /* Cross-dependencies to create more edges */
            arr1[i] = arr1[i] + (acc2 & 0xF);
            arr3[i] = arr3[i] - (acc4 >> 2);
        }
        
        volatile int marker = acc1 + acc2;
        (void)marker;
        
        /* Rotate arrays to change access pattern */
        int *temp = arr1;
        arr1 = arr2;
        arr2 = arr3;
        arr3 = arr4;
        arr4 = temp;
    }
}

int main(void) {
    /* Initialize with volatile to prevent constant propagation */
    volatile int outer_bound = 100;
    int actual_bound = outer_bound;
    
    /* Allocate and initialize arrays */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data4 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2 || !data3 || !data4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        data3[i] = rand() % 1000;
        data4[i] = rand() % 1000;
    }
    
    int sum1 = 0, sum2 = 0;
    
    /* Call the computation function */
    compute_recurrences(data1, data2, &sum1, &sum2, actual_bound);
    
    /* Second computation with more complex dependencies */
    int sum3 = 0, sum4 = 0, sum5 = 0, sum6 = 0;
    compute_multiple_recurrences(data1, data2, data3, data4, actual_bound / 2);
    
    /* Use results to prevent optimization */
    printf("Results: %d %d\n", sum1, sum2);
    printf("Array samples: %d %d %d %d\n", data1[0], data2[0], data3[0], data4[0]);
    
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    
    return 0;
}

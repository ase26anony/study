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
                          volatile int outer_bound) {
    volatile int side_effect = 0;
    
    /* Outer loop to provide multiple scheduling contexts */
    for (int j = 0; j < outer_bound; ++j) {
        int local_sum1 = *sum1;
        int local_sum2 = *sum2;
        
        /* Target inner loop with constant bound - candidate for modulo scheduling */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* First recurrence: sum reduction with loop-carried dependency */
            local_sum1 = data1[i] + local_sum1;
            
            /* Second recurrence: different operation to create separate dependency chain */
            local_sum2 = data2[i] * local_sum2 + i;
            
            /* Third recurrence: mixed operations to increase edge complexity */
            data1[i] = data1[i] - local_sum1 / 2;
        }
        
        /* Update global sums */
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

/* Alternative implementation with nested loops in main */
static void alternative_implementation(void) {
    volatile int outer_bound = 100;  /* Prevent constant propagation */
    int data1[ARRAY_SIZE];
    int data2[ARRAY_SIZE];
    int sum1 = 0, sum2 = 1;  /* sum2 starts at 1 for multiplication */
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 100;
        data2[i] = (rand() % 50) + 1;  /* Avoid zero for multiplication */
    }
    
    /* Direct nested loop structure */
    for (int j = 0; j < outer_bound; ++j) {
        volatile int prevent_opt = j;
        
        /* Inner loop with multiple recurrences */
        for (int i = 0; i < INNER_ITER; ++i) {
            /* Multiple independent recurrence chains */
            sum1 = data1[i] + sum1;              /* Simple addition chain */
            sum2 = data2[i] * sum2 - sum1;       /* Mixed operation chain */
            
            /* Another recurrence with different distance pattern */
            if (i > 0) {
                data1[i] = data1[i] + data1[i-1] * 2;  /* Distance-1 dependency */
            }
        }
        
        /* Use prevent_opt to avoid dead code elimination */
        if (prevent_opt < 0) {
            data1[0] = 0;
        }
    }
    
    printf("Results: sum1=%d, sum2=%d\n", sum1, sum2);
}

int main(void) {
    int data1[ARRAY_SIZE];
    int data2[ARRAY_SIZE];
    int sum1 = 0, sum2 = 1;  /* Start sum2 at 1 for multiplication */
    volatile int outer_bound = 100;  /* Runtime value prevents unrolling */
    
    /* Initialize arrays with random data */
    srand(42);  /* Fixed seed for reproducibility */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data1[i] = rand() % 256;
        data2[i] = (rand() % 128) + 1;  /* Ensure non-zero for multiplication */
    }
    
    /* Call the processing function */
    process_arrays(data1, data2, &sum1, &sum2, outer_bound);
    
    /* Also run alternative implementation */
    alternative_implementation();
    
    /* Print results to prevent optimization */
    printf("Final sums: %d, %d\n", sum1, sum2);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Simple LCG for pseudo-random values */
static unsigned int seed = 12345;
static unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Function containing the target loop */
int process_array(volatile int* array, int n) {
    volatile int acc1 = 0;
    volatile int acc2 = 0;
    volatile int prev = 0;
    volatile int curr = 1;
    
    /* Outer loop to provide context */
    for (int outer = 0; outer < 3; outer++) {
        /* Reset for each outer iteration */
        prev = 0;
        curr = array[0] % 256;
        
        /* Target inner loop with loop-carried dependencies */
        for (int i = 0; i < n; i++) {
            /* Multiple arithmetic operations to create scheduling complexity */
            int val = array[i];
            
            /* Recurrence 1: acc1 depends on its previous value (distance 1) */
            acc1 = acc1 + val * 3;
            
            /* Recurrence 2: prev/curr chain with distance 1 dependency */
            int temp = prev * 2;      /* Use prev from previous iteration */
            prev = curr;              /* Set prev for next iteration */
            curr = (val + temp) >> 1; /* Current value for next iteration */
            
            /* More arithmetic to increase instruction count */
            acc2 = acc2 ^ (val * prev);
            acc1 = acc1 - (curr & 0xFF);
            
            /* Additional operations for scheduling complexity */
            int scaled = val * 7;
            acc2 = acc2 + (scaled >> 2);
            acc1 = acc1 | (prev << 3);
        }
    }
    
    /* Combine results to prevent dead code elimination */
    return (acc1 & 0xFFFF) + (acc2 & 0xFFFF);
}

int main(void) {
    /* Create and initialize array with volatile elements */
    volatile int array[SIZE];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Volatile to prevent constant propagation */
    volatile int iterations = SIZE;
    
    /* Execute the processing function */
    int result = process_array(array, iterations);
    
    /* Print result to ensure side effects are observable */
    printf("Result: %d\n", result);
    
    /* Additional runs to increase execution time */
    for (int repeat = 0; repeat < 5; repeat++) {
        result += process_array(array, iterations >> 1);
    }
    
    printf("Final result: %d\n", result);
    
    return 0;
}

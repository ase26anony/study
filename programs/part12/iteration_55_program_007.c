#include <stdio.h>
#include <stdint.h>

/* Simple LCG for generating pseudo-random values */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function to create loop-carried dependencies */
int process_array(volatile int* array, int size, int iterations) {
    volatile int acc1 = 0;
    volatile int acc2 = 1;
    volatile int prev = 0;
    volatile int curr = 0;
    
    /* Outer loop to provide context */
    for (int iter = 0; iter < iterations; iter++) {
        /* Reset for each iteration */
        prev = 0;
        curr = array[0];
        
        /* Target inner loop with loop-carried dependencies */
        for (int i = 0; i < size; i++) {
            /* Distance-1 dependency: prev from iteration i-1 used in iteration i */
            int temp = prev * 3;  /* Use prev from previous iteration */
            
            /* Multiple arithmetic operations to create scheduling complexity */
            acc1 = acc1 + array[i] * 7;      /* Simple accumulation */
            acc2 = acc2 * (array[i] + 2) >> 1; /* More complex operation */
            
            /* Recurrence: acc1 depends on its own previous value */
            int recur = acc1 * 5 + temp;
            
            /* Pointer-chase like dependency chain */
            prev = curr;          /* Distance-1: prev = old curr */
            curr = array[i] + recur; /* New curr for next iteration */
            
            /* Additional operations to increase instruction count */
            int extra = (array[i] << 2) | (array[i] >> 3);
            acc1 = acc1 ^ extra;
            
            /* Another distance-1 dependency */
            int delta = curr - prev;  /* Uses both curr and prev */
            acc2 = acc2 + delta * 11;
        }
    }
    
    /* Combine results to prevent dead code elimination */
    return (acc1 & 0xFFFF) + (acc2 & 0xFFFF);
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int OUTER_ITERATIONS = 100;
    
    /* Initialize array with pseudo-random values */
    volatile int data[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Process the array multiple times */
    int result = process_array(data, ARRAY_SIZE, OUTER_ITERATIONS);
    
    /* Print result to ensure side effects are observable */
    printf("Result: %d\n", result);
    
    return 0;
}

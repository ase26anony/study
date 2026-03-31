#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function to create loop-carried dependencies */
int process_loop(volatile int* array, int size, int iter_count) {
    volatile int acc1 = 1;      /* Main accumulator with recurrence */
    volatile int acc2 = 0;      /* Secondary accumulator */
    volatile int prev = 0;       /* For distance-1 dependency */
    volatile int curr = 0;
    volatile int temp = 0;
    
    /* Outer loop to provide context */
    for (int outer = 0; outer < iter_count; outer++) {
        /* Reset for each outer iteration */
        prev = array[0];
        curr = 0;
        
        /* Target inner loop with loop-carried dependencies */
        for (int i = 1; i < size; i++) {
            /* Create distance-1 dependency: prev from iteration i-1 */
            temp = prev * 3;          /* Use prev from previous iteration */
            
            /* Main recurrence: acc1 depends on its own previous value */
            acc1 = acc1 + array[i] * 7;
            
            /* Multiple arithmetic operations to create scheduling complexity */
            acc2 = (acc2 ^ array[i]) + (temp >> 2);
            
            /* Pointer-chase like dependency chain */
            curr = array[i] + (acc1 & 0xFF);
            
            /* Distance-1 assignment for next iteration */
            prev = curr;              /* prev will be used in next iteration */
            
            /* Additional operations to increase register pressure */
            acc1 = acc1 - (curr % 17);
            acc2 = acc2 * 2 + (temp & 0x3F);
        }
    }
    
    /* Combine results to prevent dead code elimination */
    return acc1 + acc2 + prev + curr;
}

int main() {
    const int ARRAY_SIZE = 1024;
    const int OUTER_ITER = 10;
    volatile int data[ARRAY_SIZE];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Execute the loop with dependencies */
    int result = process_loop(data, ARRAY_SIZE, OUTER_ITER);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}

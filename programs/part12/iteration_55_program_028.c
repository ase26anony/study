#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function containing the target loop */
uint64_t process_array(volatile uint32_t* array, int iterations) {
    volatile uint32_t acc1 = 0;
    volatile uint32_t acc2 = 0;
    volatile uint32_t prev = 0;
    volatile uint32_t curr = 0;
    
    /* Outer loop to provide context */
    for (int outer = 0; outer < 2; outer++) {
        /* Target inner loop with loop-carried dependencies */
        for (int i = 0; i < iterations; i++) {
            /* Create distance-1 dependency: prev from iteration i-1 used here */
            uint32_t temp = prev * 3;  /* Use prev from previous iteration */
            
            /* Array access with arithmetic */
            uint32_t val = array[i] ^ 0x55AA55AA;
            
            /* Multiple arithmetic operations to create scheduling complexity */
            val = (val << 3) | (val >> 29);  /* Rotate left by 3 */
            val = val * 7 + 1;
            
            /* Recurrence: accumulator with loop-carried dependency */
            acc1 = acc1 + val * acc1;  /* acc1 depends on its previous value */
            
            /* Another recurrence with different operations */
            acc2 = (acc2 ^ val) * 13;
            
            /* Distance-1 chain: curr becomes prev for next iteration */
            prev = curr;      /* prev gets old curr value */
            curr = val + temp; /* curr gets new value */
            
            /* More arithmetic to increase instruction count */
            array[i] = (array[i] + acc1) & 0xFFFF;
        }
    }
    
    /* Combine accumulators to prevent elimination */
    return (uint64_t)acc1 << 32 | acc2;
}

int main(void) {
    /* Declare and initialize array with volatile to prevent optimization */
    volatile uint32_t array[ARRAY_SIZE];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = lcg_rand();
    }
    
    /* Volatile iteration count to prevent constant propagation */
    volatile int iterations = ARRAY_SIZE;
    
    /* Execute the processing function multiple times */
    uint64_t total_result = 0;
    for (int repeat = 0; repeat < 3; repeat++) {
        total_result ^= process_array(array, iterations);
        
        /* Modify array slightly between calls */
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            array[i] = array[i] * 3 + 1;
        }
    }
    
    /* Print result to ensure observable side effect */
    printf("Result: 0x%016llX\n", (unsigned long long)total_result);
    
    return 0;
}

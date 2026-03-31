#include <stdio.h>
#include <stdint.h>

/* Simple pseudo-random generator to create data dependencies */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Function containing the target loop */
int process_array(volatile int* array, int size, int iterations) {
    volatile int acc1 = 1;      /* First accumulator with recurrence */
    volatile int acc2 = 0;      /* Second accumulator */
    volatile int prev = 0;      /* Previous value for distance-1 dependency */
    volatile int curr = 0;      /* Current value */
    
    /* Outer loop to provide context */
    for (int outer = 0; outer < iterations; outer++) {
        /* Reset for each outer iteration */
        prev = 0;
        curr = array[0];
        
        /* Target inner loop with loop-carried dependencies */
        for (int i = 0; i < size; i++) {
            /* Create distance-1 dependency: prev from iteration i-1 used here */
            int temp = prev * 3;  /* Use prev from previous iteration */
            
            /* Main recurrence: acc1 depends on its own previous value */
            acc1 = acc1 * 7 + array[i];
            
            /* Complex arithmetic to create scheduling opportunities */
            acc2 = (acc2 << 2) + (temp & 0xFF);
            
            /* Multiple operations to increase dataflow graph complexity */
            int mult_result = array[i] * acc1;
            int shift_result = mult_result >> 3;
            
            /* Update distance-1 chain for next iteration */
            prev = curr;
            curr = array[i] + shift_result;
            
            /* Additional arithmetic to prevent simplification */
            acc1 = acc1 ^ (curr & 0x7F);
            acc2 = acc2 + (prev % 256);
        }
    }
    
    /* Combine accumulators to prevent dead code elimination */
    return acc1 + acc2;
}

int main() {
    const int ARRAY_SIZE = 1024;
    const int OUTER_ITERATIONS = 100;
    volatile int data[ARRAY_SIZE];
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = lcg(&seed) % 1000;
    }
    
    /* Execute the target function */
    int result = process_array(data, ARRAY_SIZE, OUTER_ITERATIONS);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}

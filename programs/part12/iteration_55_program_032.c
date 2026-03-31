#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Function containing the target loop */
int process_array(volatile int* array, int size, int iterations) {
    volatile int sum = 0;
    volatile int prev = 0;
    volatile int curr = 1;
    volatile int temp;
    
    /* Outer loop to provide context */
    for (int iter = 0; iter < iterations; iter++) {
        /* Reset for each outer iteration */
        prev = 0;
        curr = array[0];
        sum = 0;
        
        /* TARGET INNER LOOP - designed for modulo scheduling */
        for (int i = 0; i < size; i++) {
            /* Loop-carried dependency: prev from iteration i-1 used here */
            temp = prev * 3;           /* Use prev from previous iteration */
            
            /* Multiple arithmetic operations creating data flow */
            int val = array[i];
            curr = val + (temp >> 2);  /* curr depends on temp */
            
            /* Recurrence: accumulator with loop-carried dependency */
            sum = sum + curr * 2;      /* sum depends on previous sum */
            
            /* Distance-1 dependency: prev will be used next iteration */
            prev = curr & 0xFF;        /* prev for next iteration */
            
            /* Additional operations to increase scheduling complexity */
            sum = sum ^ (val * 5);
            curr = curr + (i & 0x3);   /* Mix in loop index */
        }
    }
    
    return sum;
}

int main() {
    const int ARRAY_SIZE = 1024;
    const int OUTER_ITERATIONS = 10;
    
    /* Initialize array with pseudo-random values */
    volatile int array[ARRAY_SIZE];
    uint32_t seed = 42;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = lcg(&seed) % 100;
    }
    
    /* Execute the target function */
    int result = process_array(array, ARRAY_SIZE, OUTER_ITERATIONS);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}

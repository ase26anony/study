#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function containing the target loop */
int process_array(volatile int* array, int size, int iterations) {
    volatile int acc1 = 0, acc2 = 1, acc3 = 0;
    volatile int prev_val = 0, curr_val = 0;
    volatile int* volatile ptr = array; /* volatile pointer to volatile data */
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Reset for each outer iteration */
        acc1 = 0;
        acc2 = 1;
        prev_val = 0;
        ptr = array;
        
        /* TARGET INNER LOOP - designed for modulo scheduling */
        for (int i = 0; i < size; i++) {
            /* 1. Loop-carried dependency with distance 1 */
            prev_val = curr_val;                /* Use in next iteration */
            curr_val = *ptr + i;                /* Def here, use next iteration */
            
            /* 2. Multiple arithmetic operations creating data flow */
            int temp1 = acc1 * 3 + prev_val;    /* Uses prev_val from prev iteration */
            int temp2 = (acc2 << 2) | (i & 0xF);
            int temp3 = temp1 ^ temp2;
            
            /* 3. Recurrence/accumulator with loop-carried dependency */
            acc1 = acc3 + temp3;                /* acc3 from previous iteration */
            acc2 = acc1 * 7 - temp2;
            acc3 = acc2 >> 1;                   /* Will be used in next iteration */
            
            /* 4. Pointer arithmetic with volatile to prevent optimization */
            ptr++;
            
            /* 5. Additional operations to increase scheduling complexity */
            if (i & 1) {
                acc1 = acc1 + (temp3 & 0xFF);
            } else {
                acc1 = acc1 - (temp2 % 256);
            }
        }
    }
    
    /* Combine results to ensure all computations are used */
    return acc1 + acc2 + acc3 + prev_val + curr_val;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int OUTER_ITERATIONS = 100;
    
    /* Initialize array with pseudo-random values */
    volatile int array[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Execute the target function */
    int result = process_array(array, ARRAY_SIZE, OUTER_ITERATIONS);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}

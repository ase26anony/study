#include <stdio.h>
#include <stdlib.h>

/* Simple pseudo-random generator to create data dependencies */
static unsigned int lcg_seed = 123456789;
static inline unsigned int lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function containing the target loop */
int process_array(volatile int* array, int size, int iterations) {
    volatile int acc1 = 0, acc2 = 0;
    volatile int prev_val = 0, curr_val = 0;
    volatile int* volatile ptr = array; /* volatile pointer to volatile data */
    
    /* Outer loop to provide context */
    for (int iter = 0; iter < iterations; iter++) {
        /* Reset for each iteration */
        acc1 = 1;
        acc2 = array[0];
        prev_val = array[0];
        
        /* TARGET INNER LOOP - designed for modulo scheduling */
        for (int i = 1; i < size; i++) {
            /* Create loop-carried dependency with distance 1 */
            curr_val = array[i];           /* Current value */
            
            /* Recurrence 1: acc1 depends on its previous value (distance 1) */
            acc1 = acc1 * 3 + curr_val;    /* acc1[i] = acc1[i-1] * 3 + curr_val */
            
            /* Recurrence 2: acc2 depends on prev_val from previous iteration */
            acc2 = acc2 + prev_val * 2;    /* acc2[i] = acc2[i-1] + prev_val[i-1] * 2 */
            
            /* Multiple arithmetic operations to create scheduling complexity */
            int temp1 = curr_val << 2;     /* Shift operation */
            int temp2 = temp1 + i;         /* Add index */
            int temp3 = temp2 * acc1;      /* Multiply with accumulator */
            
            /* Update prev_val for next iteration (distance 1 dependency) */
            prev_val = temp3 & 0xFF;       /* Mask operation */
            
            /* Additional array access with pointer arithmetic */
            ptr = array + (i % 16);
            acc1 = acc1 ^ (*ptr);          /* XOR operation */
        }
    }
    
    /* Combine results to prevent dead code elimination */
    return (acc1 + acc2) & 0x7FFFFFFF;
}

/* Another function with different recurrence pattern */
int process_linked(volatile int* data, int size) {
    volatile int sum = 0;
    volatile int carry = 0;
    
    /* Loop with multiple interleaved dependencies */
    for (int i = 0; i < size; i++) {
        /* Chain of operations creating distance-1 dependencies */
        int val1 = data[i] + carry;        /* Uses carry from previous iteration */
        int val2 = val1 * val1;            /* Square operation */
        int val3 = val2 >> 3;              /* Shift operation */
        
        /* Update carry for next iteration */
        carry = val3 & 1;                  /* Extract LSB for next iteration */
        
        /* Accumulate with previous sum */
        sum = sum + val3;                  /* sum[i] = sum[i-1] + val3 */
        
        /* Additional operation mixing index and value */
        sum = sum ^ (i * 7);               /* XOR with scaled index */
    }
    
    return sum;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int OUTER_ITERATIONS = 10;
    
    /* Allocate and initialize array with pseudo-random values */
    volatile int* array = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with values that create data dependencies */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = lcg_rand() % 1000;
    }
    
    /* Execute the target function multiple times */
    int total_result = 0;
    for (int run = 0; run < 5; run++) {
        int result1 = process_array(array, ARRAY_SIZE, OUTER_ITERATIONS);
        int result2 = process_linked(array, ARRAY_SIZE);
        total_result += result1 + result2;
        
        /* Modify array slightly between runs to vary execution */
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            array[i] = array[i] + run;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", total_result);
    
    free((void*)array);
    return 0;
}

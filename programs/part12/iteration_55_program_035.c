#include <stdio.h>
#include <stdlib.h>

/* Simple pseudo-random generator to create data dependencies */
static unsigned int lcg_seed = 123456789;
static inline unsigned int lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function to create a loop with complex dependencies */
int process_loop(volatile int* array, int size, int iterations) {
    volatile int acc1 = 0;
    volatile int acc2 = 1;
    volatile int prev = 0;
    volatile int curr = 0;
    volatile int temp1, temp2, temp3;
    
    /* Outer loop to provide context */
    for (int outer = 0; outer < iterations; outer++) {
        /* Reset accumulators each outer iteration */
        acc1 = outer;
        acc2 = 1;
        prev = 0;
        curr = array[0];
        
        /* TARGET INNER LOOP - designed for modulo scheduling */
        for (int i = 0; i < size; i++) {
            /* Create loop-carried dependency: prev depends on curr from previous iteration */
            prev = curr;  /* distance1_uses: prev used in next iteration */
            curr = array[i] & 0xFF;  /* Current value */
            
            /* Multiple arithmetic operations with dependencies */
            temp1 = prev * acc1;      /* Uses prev from previous iteration */
            temp2 = curr * acc2;      /* Uses current value */
            temp3 = temp1 + temp2;    /* Combines both */
            
            /* Recurrence: acc1 depends on its previous value */
            acc1 = acc1 + temp3;      /* True loop-carried dependency */
            
            /* Another recurrence with different operations */
            acc2 = (acc2 << 1) | (temp3 & 1);  /* Shift and bitwise OR */
            
            /* More operations to increase scheduling complexity */
            temp1 = temp1 ^ temp2;    /* XOR operation */
            temp2 = temp3 * i;        /* Multiply by loop index */
            acc1 = acc1 ^ temp2;      /* Modify accumulator again */
        }
    }
    
    /* Combine results to prevent dead code elimination */
    return acc1 + acc2 + prev + curr;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int OUTER_ITERATIONS = 100;
    
    /* Initialize array with pseudo-random values */
    volatile int* array = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill array with values that create data dependencies */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = lcg_rand() % 1000;
    }
    
    /* Execute the target loop */
    int result = process_loop(array, ARRAY_SIZE, OUTER_ITERATIONS);
    
    /* Print result to ensure side effects are observable */
    printf("Result: %d\n", result);
    
    /* Verify computation */
    printf("Verification: array[0] = %d, array[%d] = %d\n", 
           array[0], ARRAY_SIZE-1, array[ARRAY_SIZE-1]);
    
    free((void*)array);
    return 0;
}

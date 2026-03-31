#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand() {
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
        /* Reset for each outer iteration */
        prev = 0;
        curr = array[0];
        
        /* Target inner loop with loop-carried dependencies */
        for (int i = 0; i < size; i++) {
            /* Create distance-1 dependency: prev from iteration i-1 */
            int temp = prev * 3;  /* Use prev from previous iteration */
            
            /* Multiple arithmetic operations to create scheduling complexity */
            acc1 = acc1 + array[i] * 7;      /* Simple accumulation */
            acc2 = acc2 * (array[i] + 2) & 0xFF; /* Modulo operation */
            
            /* Recurrence with distance-1: prev = curr, curr = new value */
            prev = curr;                     /* Becomes prev in next iteration */
            curr = array[i] + temp;          /* New curr for next iteration */
            
            /* More operations to increase scheduling pressure */
            acc1 = (acc1 << 1) | (acc1 >> 31);  /* Rotate */
            acc2 = acc2 ^ (array[i] * 13);      /* XOR operation */
            
            /* Another recurrence: acc1 depends on its previous value */
            acc1 = acc1 + (acc1 % 17);          /* Self-dependency */
        }
    }
    
    /* Combine accumulators to prevent dead code elimination */
    return acc1 + acc2 + prev + curr;
}

int main() {
    const int ARRAY_SIZE = 1024;
    const int OUTER_ITERATIONS = 100;
    
    /* Initialize array with pseudo-random values */
    volatile int data[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Process the array multiple times */
    int result = process_array(data, ARRAY_SIZE, OUTER_ITERATIONS);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}

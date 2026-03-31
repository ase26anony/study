#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Simple pseudo-random generator to create data dependencies */
static inline unsigned int lcg(unsigned int prev) {
    return (1103515245u * prev + 12345u) & 0x7fffffffu;
}

/* Function containing the target loop */
int process_array(volatile int* array, int n) {
    volatile int acc1 = 1;      /* Volatile to prevent optimization */
    volatile int acc2 = 0;
    volatile int prev_val = 0;
    volatile int curr_val = 0;
    volatile int* volatile ptr = array; /* Volatile pointer */
    
    /* Outer loop to provide context */
    for (int outer = 0; outer < 3; outer++) {
        /* Reset for each outer iteration */
        acc1 = 1;
        acc2 = 0;
        prev_val = 0;
        curr_val = array[0];
        ptr = array;
        
        /* Target inner loop with loop-carried dependencies */
        for (int i = 0; i < n; i++) {
            /* Recurrence 1: acc1 depends on its previous value */
            acc1 = acc1 * (*ptr + 1);
            
            /* Recurrence 2: acc2 accumulates with dependency chain */
            acc2 = acc2 + (acc1 >> 2);
            
            /* Distance-1 dependency: prev_val used in next iteration */
            int temp = curr_val;
            curr_val = (*ptr * 3) + (prev_val & 0x1F); /* prev_val used here */
            prev_val = temp; /* Set for next iteration */
            
            /* Additional arithmetic operations */
            acc1 = acc1 ^ (curr_val << 1);
            acc2 = acc2 | (prev_val * 7);
            
            /* Pointer arithmetic with dependency */
            ptr = array + ((i + 1) % n);
            
            /* More operations to increase scheduling complexity */
            acc1 = (acc1 * 13) - (acc2 & 0xFF);
            acc2 = (acc2 + *ptr) ^ (acc1 >> 3);
        }
    }
    
    /* Combine results to ensure all computations are used */
    return (acc1 + acc2 + prev_val + curr_val);
}

int main() {
    /* Create and initialize array with pseudo-random values */
    volatile int array[SIZE];
    unsigned int seed = 42;
    
    for (int i = 0; i < SIZE; i++) {
        seed = lcg(seed);
        array[i] = (int)(seed % 1000);
    }
    
    /* Volatile to prevent loop removal */
    volatile int iterations = SIZE;
    
    /* Execute the target loop */
    int result = process_array(array, iterations);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional verification to ensure loop executed */
    volatile int check = 0;
    for (int i = 0; i < SIZE; i++) {
        check += array[i];
    }
    printf("Array sum: %d\n", check);
    
    return 0;
}

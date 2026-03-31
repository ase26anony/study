#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Simple pseudo-random generator to create non-constant data */
static inline unsigned int lcg(unsigned int *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Function containing the target loop */
int process_loop(volatile int *array, int count) {
    volatile int acc1 = 0;
    volatile int acc2 = 1;
    volatile int prev = 0;
    volatile int curr = 1;
    
    /* 
     * Target inner loop with:
     * 1. Loop-carried dependencies (recurrence)
     * 2. Distance-1 uses (prev = curr pattern)
     * 3. Multiple arithmetic operations
     * 4. Array accesses
     */
    for (int i = 0; i < count; i++) {
        /* Distance-1 dependency: curr from previous iteration becomes prev */
        prev = curr;  /* Used in next iteration */
        
        /* Array access with recurrence */
        curr = array[i] * 3 + 7;
        
        /* Loop-carried accumulator with non-trivial arithmetic */
        acc1 = acc1 + prev * 2;  /* Uses prev from previous iteration */
        
        /* Another recurrence with different operations */
        acc2 = (acc2 * 3 + curr) >> 1;
        
        /* Additional arithmetic to increase scheduling complexity */
        int temp = (prev + curr) * (acc1 & 0xFF);
        acc1 = acc1 ^ (temp & 0xFFFF);
    }
    
    /* Mix results to prevent dead code elimination */
    return (acc1 + acc2) & 0x7FFFFFFF;
}

/* Outer loop to provide context */
void outer_loop(volatile int *array, int outer_count, int inner_count) {
    volatile int total = 0;
    
    for (int j = 0; j < outer_count; j++) {
        /* Slightly modify array each outer iteration */
        for (int k = 0; k < SIZE; k++) {
            array[k] = (array[k] * 3 + j) & 0xFFF;
        }
        
        /* Call the target inner loop multiple times */
        total += process_loop(array, inner_count);
    }
    
    /* Print result to ensure side effects are observable */
    printf("Result: %d\n", total);
}

int main() {
    /* Create and initialize array with volatile to prevent optimization */
    volatile int *array = (volatile int*)malloc(SIZE * sizeof(int));
    if (!array) return 1;
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg(&seed) % 1000;
    }
    
    /* Execute the loop structure */
    outer_loop(array, 10, SIZE);
    
    free((void*)array);
    return 0;
}

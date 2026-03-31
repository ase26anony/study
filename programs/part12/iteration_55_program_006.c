#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE 1024

/* Simple pseudo-random generator to avoid library calls */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function containing the target loop */
int process_array(volatile int* array, int n, int init) {
    volatile int acc1 = init;      /* Volatile to prevent optimization */
    volatile int acc2 = 0;
    volatile int prev = 0;
    volatile int curr = array[0];
    
    /* 
     * Target inner loop with:
     * 1. Loop-carried dependency: prev = curr; curr = ... (distance=1)
     * 2. Recurrence: acc1 depends on its previous value
     * 3. Multiple arithmetic operations
     * 4. Array accesses
     */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependency chain */
        prev = curr;                    /* Used in next iteration */
        curr = array[i] & 0xFF;         /* Defines curr for next iteration */
        
        /* Recurrence with accumulator */
        acc1 = acc1 + (prev * 3);       /* True loop-carried dependency */
        
        /* Multiple arithmetic operations */
        int temp = array[i] * 7;
        temp = temp >> 2;
        temp = temp + i;
        
        /* Second accumulator with different operations */
        acc2 = acc2 ^ temp;             /* Another loop-carried dependency */
        
        /* More operations to increase scheduling complexity */
        int tmp2 = array[i] + acc1;
        tmp2 = tmp2 * 2;
        acc1 = acc1 - (tmp2 & 1);       /* Additional use of acc1 */
    }
    
    /* Mix results to prevent dead code elimination */
    return (acc1 & 0xFFFF) + (acc2 & 0xFFFF);
}

/* Outer loop to provide context */
void outer_loop(volatile int* array, int outer_iterations) {
    volatile int total = 0;
    
    for (int iter = 0; iter < outer_iterations; iter++) {
        /* Vary the initialization to prevent pattern recognition */
        int init = (iter * 37) & 0xFF;
        
        /* Call the function with the target loop */
        int result = process_array(array, SIZE, init);
        
        total += result;
        
        /* Modify array slightly between iterations */
        array[iter % SIZE] = lcg_rand() & 0xFF;
    }
    
    /* Ensure side effect */
    printf("Total result: %d\n", total);
}

int main() {
    /* Allocate and initialize array with volatile elements */
    volatile int* array = (volatile int*)malloc(SIZE * sizeof(int));
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg_rand() & 0xFF;
    }
    
    /* Execute the outer loop multiple times */
    outer_loop(array, 10);
    
    /* Cleanup */
    free((void*)array);
    
    return 0;
}

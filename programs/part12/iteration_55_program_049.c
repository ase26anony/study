#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

// Simple pseudo-random generator to create data dependencies
static unsigned int seed = 12345;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

// Function containing the target loop
void process_loop(volatile int* array, int n) {
    volatile int acc1 = 0;
    volatile int acc2 = 0;
    volatile int prev = 0;
    volatile int curr = 0;
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset accumulators each outer iteration
        acc1 = 1;
        acc2 = array[0];
        prev = array[0];
        
        // Target inner loop with loop-carried dependencies
        for (int i = 1; i < n; i++) {
            // Recurrence 1: acc1 depends on its previous value (distance 1)
            acc1 = acc1 * 3 + array[i];
            
            // Recurrence 2: acc2 depends on acc1 from previous iteration
            // This creates a chain: acc1(i-1) -> acc2(i)
            acc2 = acc1 + acc2 * 2;
            
            // Distance-1 use pattern: prev = curr from previous iteration
            // curr is computed, then used as prev in next iteration
            curr = array[i] * 7 + prev;
            prev = curr >> 1;  // Use shift to add operation variety
            
            // Additional arithmetic to increase scheduling complexity
            int temp = (acc1 & 0xFF) * (acc2 & 0xFF);
            array[i] = (array[i] + temp) & 0xFFFF;
        }
        
        // Use results to prevent dead code elimination
        printf("Iteration %d: acc1=%d, acc2=%d, prev=%d\n", 
               outer, acc1, acc2, prev);
    }
}

int main() {
    // Create and initialize array with volatile-qualified pointer
    volatile int* array = (volatile int*)malloc(SIZE * sizeof(int));
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with pseudo-random values
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg_rand() % 100;
    }
    
    // Process the loop multiple times
    process_loop(array, SIZE);
    
    // Additional processing to ensure loop execution
    volatile int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += array[i];
    }
    
    printf("Final sum: %d\n", sum);
    
    free((void*)array);
    return 0;
}

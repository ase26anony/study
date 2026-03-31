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
int process_array(volatile int* array, int n) {
    volatile int acc1 = 1;      // Accumulator with recurrence
    volatile int acc2 = 0;      // Secondary accumulator
    volatile int prev = 0;      // For distance-1 dependency
    volatile int curr = 0;
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        acc1 = 1;
        acc2 = 0;
        prev = 0;
        curr = array[0];
        
        // Target inner loop with loop-carried dependencies
        for (int i = 0; i < n; i++) {
            // Distance-1 dependency: prev from previous iteration
            int temp = prev * 3;           // Use prev from i-1
            
            // Recurrence on acc1 (loop-carried true dependency)
            acc1 = acc1 + array[i] * 7;    // acc1 depends on previous acc1
            
            // Multiple arithmetic operations
            acc2 = acc2 ^ (array[i] << 2); // Different operation
            curr = array[i] * 5 + temp;    // Mix of operations
            
            // Create distance-1 use for next iteration
            prev = curr & 0xFF;            // prev used in next iteration
            
            // More operations to increase scheduling complexity
            acc1 = (acc1 >> 1) | (acc1 << 31); // Rotate
            acc2 = acc2 + (curr % 17);         // Modulo operation
        }
    }
    
    // Combine results to prevent dead code elimination
    return (acc1 + acc2 + prev + curr) & 0x7FFFFFFF;
}

int main() {
    // Allocate and initialize array with volatile-like behavior
    volatile int* array = (volatile int*)malloc(SIZE * sizeof(int));
    
    // Initialize with pseudo-random values
    for (int i = 0; i < SIZE; i++) {
        array[i] = (lcg_rand() % 100) - 50; // Values between -50 and 49
    }
    
    // Process the array multiple times
    int total = 0;
    for (int iter = 0; iter < 5; iter++) {
        total += process_array(array, SIZE);
        
        // Modify array slightly each iteration
        for (int i = 0; i < SIZE; i += 7) {
            array[i] += iter;
        }
    }
    
    // Print result to ensure side effects are observable
    printf("Result: %d\n", total);
    
    free((void*)array);
    return 0;
}

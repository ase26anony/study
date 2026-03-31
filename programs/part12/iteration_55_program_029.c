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
int process_array(volatile int* arr, int n) {
    volatile int acc1 = 1;      // Accumulator with recurrence
    volatile int acc2 = 0;      // Secondary accumulator
    volatile int prev = 0;      // Previous value for distance-1 dependency
    volatile int curr = 0;      // Current value
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        acc1 = 1;
        acc2 = 0;
        prev = arr[0];
        
        // Target inner loop with loop-carried dependencies
        for (int i = 0; i < n; i++) {
            // Distance-1 dependency: prev from iteration i-1 used here
            curr = arr[i];
            
            // Recurrence 1: acc1 depends on its previous value
            // Creates true loop-carried dependency with distance 1
            acc1 = acc1 * (curr + 1);
            
            // Recurrence 2: acc2 has more complex recurrence
            // Mix of operations to create scheduling complexity
            acc2 = (acc2 << 1) + (prev * 3);
            
            // Multiple arithmetic operations
            int temp1 = curr * 7;
            int temp2 = temp1 >> 2;
            int temp3 = temp2 + (i & 0xF);
            
            // Update prev for next iteration's distance-1 use
            prev = temp3;
            
            // Additional operation with array access
            acc1 = acc1 - (arr[(i + 1) % n] & 0x7);
        }
    }
    
    // Combine accumulators to prevent elimination
    return acc1 + acc2;
}

int main() {
    // Create and initialize array with volatile pointer
    volatile int* array = (volatile int*)malloc(SIZE * sizeof(int));
    
    // Fill array with pseudo-random values
    for (int i = 0; i < SIZE; i++) {
        array[i] = (lcg_rand() % 256) - 128;  // Values between -128 and 127
    }
    
    // Process the array multiple times
    int total_result = 0;
    for (int repeat = 0; repeat < 5; repeat++) {
        total_result += process_array(array, SIZE);
        
        // Modify array slightly between repetitions
        // to prevent complete optimization
        for (int i = 0; i < SIZE; i += 64) {
            array[i] += repeat;
        }
    }
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", total_result);
    
    free((void*)array);
    return 0;
}

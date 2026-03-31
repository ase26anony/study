#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

// Simple pseudo-random generator to create data dependencies
static unsigned int simple_rand(unsigned int seed) {
    return seed * 1103515245 + 12345;
}

// Function containing the target loop
unsigned int process_array(volatile int* array, int n) {
    volatile unsigned int acc1 = 1;      // Accumulator with recurrence
    volatile unsigned int acc2 = 0;      // Secondary accumulator
    volatile unsigned int prev = 0;      // Previous value for distance-1 dependency
    volatile unsigned int curr = array[0]; // Current value
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        acc1 = 1;
        acc2 = 0;
        prev = 0;
        curr = array[0];
        
        // Target inner loop with loop-carried dependencies
        for (int i = 0; i < n; i++) {
            // Create distance-1 dependency: prev used in next iteration
            prev = curr;                    // Distance-1: prev_i = curr_i
            curr = array[i];                // curr_i = array[i]
            
            // Recurrence: acc1 depends on its previous value
            acc1 = acc1 * 3 + array[i];     // True loop-carried dependency
            
            // Multiple arithmetic operations to create scheduling complexity
            unsigned int temp1 = acc1 << 2;  // Shift operation
            unsigned int temp2 = prev * 7;   // Use prev from previous iteration
            unsigned int temp3 = temp1 + temp2;
            
            // Another recurrence with different operation
            acc2 = (acc2 + temp3) ^ 0x5A5A;  // XOR creates data dependency
            
            // More operations to increase instruction count
            unsigned int temp4 = (curr * acc1) >> 1;
            acc2 = acc2 + temp4;
            
            // Use both accumulators in computation
            unsigned int temp5 = (acc1 & 0xFF) * (acc2 & 0xFF);
            acc1 = acc1 ^ temp5;
        }
    }
    
    // Combine results to prevent dead code elimination
    return acc1 + acc2;
}

int main() {
    // Create and initialize array with pseudo-random values
    volatile int array[SIZE];
    unsigned int seed = 42;
    
    for (int i = 0; i < SIZE; i++) {
        seed = simple_rand(seed);
        array[i] = (seed % 256) - 128;  // Values between -128 and 127
    }
    
    // Process the array multiple times
    unsigned int total_result = 0;
    for (int repeat = 0; repeat < 5; repeat++) {
        total_result += process_array(array, SIZE);
        
        // Modify array slightly each time to create different execution paths
        for (int i = 0; i < SIZE; i += 7) {
            array[i] = array[i] + repeat;
        }
    }
    
    // Print result to prevent optimization
    printf("Result: %u\n", total_result);
    
    return 0;
}

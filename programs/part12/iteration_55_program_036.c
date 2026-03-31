#include <stdio.h>
#include <stdint.h>

#define SIZE 1024

// Simple LCG for generating pseudo-random values
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

// Function to create loop-carried dependencies
int process_array(volatile int* array, int n) {
    volatile int acc1 = 0;
    volatile int acc2 = 0;
    volatile int prev = 0;
    volatile int curr = 0;
    
    // Create a recurrence with distance-1 dependencies
    for (int i = 0; i < n; i++) {
        // Multiple arithmetic operations to create scheduling complexity
        int val = array[i];
        
        // Distance-1 dependency: prev from previous iteration
        int temp = val * prev;  // Uses prev from iteration i-1
        
        // Accumulator recurrence (loop-carried)
        acc1 = acc1 + temp;     // acc1 depends on acc1 from previous iteration
        
        // Another distance-1 dependency chain
        curr = val >> 3;        // Current value
        int diff = curr - prev; // Uses prev from iteration i-1
        acc2 = acc2 ^ diff;     // Another accumulator
        
        // Update for next iteration's distance-1 use
        prev = curr;            // prev for next iteration (distance=1)
        
        // More arithmetic to increase instruction count
        acc1 = acc1 * 3;
        acc2 = acc2 + (val & 0xFF);
    }
    
    // Combine results to prevent dead code elimination
    return acc1 + acc2;
}

// Outer loop to provide context
void outer_loop(volatile int* array, int outer_iterations) {
    volatile int total = 0;
    
    for (int iter = 0; iter < outer_iterations; iter++) {
        // Modify array slightly each outer iteration
        for (int i = 0; i < SIZE; i++) {
            array[i] = (array[i] + iter) & 0xFFFF;
        }
        
        // Call the target inner loop
        total += process_array(array, SIZE);
    }
    
    // Print to ensure observable side effect
    printf("Final total: %d\n", total);
}

int main() {
    // Initialize array with volatile to prevent optimization
    volatile int array[SIZE];
    
    // Fill array with pseudo-random values
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg_rand() & 0xFFFF;
    }
    
    // Execute the nested loops
    outer_loop(array, 10);
    
    return 0;
}

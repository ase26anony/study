#include <stdio.h>
#include <stdint.h>

#define SIZE 1024

// Simple pseudo-random generator to create non-constant data
static uint32_t lcg(uint32_t x) {
    return (1103515245 * x + 12345) & 0x7fffffff;
}

// Function containing the target loop
int process_array(volatile int* array, int n) {
    volatile int acc1 = 1;      // First accumulator with recurrence
    volatile int acc2 = 0;      // Second accumulator
    volatile int prev = 0;      // Previous value for distance-1 dependency
    volatile int curr = 0;      // Current value
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        acc1 = 1;
        acc2 = 0;
        prev = 0;
        
        // Target inner loop with loop-carried dependencies
        for (int i = 0; i < n; i++) {
            // Create distance-1 dependency: prev from iteration i-1 used here
            curr = array[i];
            
            // Recurrence 1: acc1 depends on its previous value (iteration i-1)
            // This creates a loop-carried dependency with distance 1
            acc1 = acc1 * 3 + curr;
            
            // Recurrence 2: acc2 depends on acc1 from current iteration
            // and its own previous value from iteration i-1
            acc2 = (acc2 + acc1) >> 1;
            
            // Multiple arithmetic operations to create scheduling complexity
            int temp1 = curr * 7;
            int temp2 = temp1 + (prev * 5);  // Uses prev from iteration i-1
            int temp3 = temp2 << 2;
            
            // Update prev for next iteration (distance-1 dependency)
            prev = temp3 & 0xFF;
            
            // More operations to increase instruction count
            acc1 = acc1 ^ (temp3 & 0xF);
            acc2 = acc2 + (temp1 % 17);
        }
    }
    
    // Combine accumulators to produce observable result
    return acc1 + acc2;
}

int main() {
    // Declare and initialize array with pseudo-random values
    volatile int array[SIZE];
    uint32_t seed = 42;
    
    for (int i = 0; i < SIZE; i++) {
        seed = lcg(seed);
        array[i] = (int)(seed % 100);
    }
    
    // Process the array with the target loop
    int result = process_array(array, SIZE);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional test with different sizes
    volatile int small_array[16];
    for (int i = 0; i < 16; i++) {
        small_array[i] = i * 2;
    }
    
    int result2 = process_array(small_array, 16);
    printf("Result2: %d\n", result2);
    
    return 0;
}

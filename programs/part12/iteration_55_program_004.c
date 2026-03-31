#include <stdio.h>
#include <stdint.h>

#define SIZE 1024

// Simple pseudo-random generator to create non-constant data
static uint32_t lcg(uint32_t x) {
    return (1103515245u * x + 12345u) & 0x7fffffffu;
}

// Function containing the target loop
int process_array(volatile int* array, int n, int init) {
    volatile int acc1 = init;      // Primary accumulator with recurrence
    volatile int acc2 = 0;         // Secondary accumulator
    volatile int prev = 0;         // For distance-1 dependency
    volatile int curr = array[0];  // Current value
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        acc1 = init + outer;
        acc2 = 0;
        prev = 0;
        curr = array[0];
        
        // Target inner loop with loop-carried dependencies
        for (int i = 0; i < n; i++) {
            // Create distance-1 dependency chain: prev from previous iteration
            int temp = prev * 3;  // Uses prev from iteration i-1
            
            // Multiple arithmetic operations to create scheduling complexity
            acc1 = acc1 + array[i] * 7;  // Recurrence: acc1 depends on previous acc1
            acc2 = acc2 ^ (array[i] << 2);
            
            // More operations mixing results
            int mix = (acc1 & 0xFF) * (acc2 & 0xFF);
            temp = temp + (mix >> 4);
            
            // Update distance-1 variables for next iteration
            prev = curr;          // prev in iteration i becomes curr for iteration i+1
            curr = array[i] + temp;
            
            // Additional arithmetic to increase instruction count
            acc1 = acc1 - (curr & 1);
            acc2 = acc2 | (prev & 0xF);
        }
    }
    
    // Combine results to prevent dead code elimination
    return acc1 + acc2 + prev + curr;
}

int main() {
    // Create and initialize array with pseudo-random values
    volatile int array[SIZE];
    uint32_t seed = 42;
    
    for (int i = 0; i < SIZE; i++) {
        seed = lcg(seed);
        array[i] = (int)(seed % 100);
    }
    
    // Process the array multiple times
    int total = 0;
    for (int iter = 0; iter < 5; iter++) {
        total += process_array(array, SIZE, iter * 10);
    }
    
    // Print result to ensure side effects are observable
    printf("Result: %d\n", total);
    
    return 0;
}

#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1024

// Simple pseudo-random generator to create non-constant data
static uint32_t lcg(uint32_t x) {
    return (1103515245 * x + 12345) & 0x7fffffff;
}

// Function to create loop-carried dependencies
int process_loop(volatile int* array, int n) {
    volatile int sum = 1;        // Volatile to prevent optimization
    volatile int prev = 0;       // Previous value for distance-1 dependency
    volatile int curr = array[0]; // Current value
    
    // Tight inner loop with loop-carried dependencies
    for (int i = 0; i < n; i++) {
        // Create distance-1 dependency: prev from previous iteration
        int temp = prev * 3;      // Use prev from iteration i-1
        
        // Multiple arithmetic operations to create scheduling complexity
        curr = array[i];          // Array access
        sum = sum + curr * 7;     // Recurrence: sum depends on previous sum
        sum = sum ^ (temp << 2);  // More operations
        sum = sum + (sum >> 3);   // Shift operation
        
        // Setup for next iteration's distance-1 dependency
        prev = curr + i;          // prev will be used in next iteration
        
        // Additional operations to increase register pressure
        int extra = sum * prev;
        sum = sum + (extra & 0xFF);
    }
    
    return sum;
}

// Outer loop to provide context
void outer_loop(volatile int* data, int outer_iter, int inner_size) {
    volatile int total = 0;
    
    for (int j = 0; j < outer_iter; j++) {
        // Process inner loop multiple times
        int result = process_loop(data + j * 64, inner_size);
        total = total + result;
        
        // Modify data slightly for next outer iteration
        for (int k = 0; k < 64; k++) {
            data[j * 64 + k] = data[j * 64 + k] + 1;
        }
    }
    
    // Prevent dead code elimination
    printf("Total: %d\n", total);
}

int main() {
    // Create and initialize array with pseudo-random values
    volatile int data[ARRAY_SIZE];
    uint32_t seed = 42;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        seed = lcg(seed);
        data[i] = (int)(seed % 100);
    }
    
    // Execute the loop structure
    outer_loop(data, 8, 128);
    
    return 0;
}

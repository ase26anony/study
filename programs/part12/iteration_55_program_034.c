#include <stdio.h>
#include <stdint.h>

#define SIZE 1024

// Simple pseudo-random generator to create non-constant data
static uint32_t lcg(uint32_t x) {
    return (1103515245 * x + 12345) & 0x7fffffff;
}

// Function containing the target loop
int process_array(volatile int* array, int n, int init) {
    volatile int sum = init;          // Volatile to prevent optimization
    volatile int prev = 0;            // For distance-1 dependency
    volatile int curr = array[0];     // Initial value
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        prev = 0;
        curr = array[0];
        sum = init + outer;
        
        // Target inner loop with loop-carried dependencies
        for (int i = 0; i < n; i++) {
            // Create distance-1 dependency: prev from iteration i-1
            int temp = prev * 2;      // Use prev from previous iteration
            
            // Multiple arithmetic operations to create scheduling complexity
            int val = array[i];
            int prod = val * 3;
            int shifted = prod >> 1;
            
            // Recurrence: sum depends on previous sum (loop-carried)
            sum = sum + shifted + temp;
            
            // Update for distance-1 dependency chain
            prev = curr;              // prev gets value from current iteration
            curr = val + i;           // curr computed for next iteration
            
            // More operations to increase instruction count
            sum = sum ^ (val & 0xFF);
            sum = sum + (i % 8);      // Simple operation
        }
    }
    
    return sum;
}

int main() {
    volatile int array[SIZE];
    
    // Initialize with pseudo-random values
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = lcg(seed);
        array[i] = (int)(seed % 100);
    }
    
    // Process the array multiple times
    volatile int total = 0;
    for (int iter = 0; iter < 5; iter++) {
        total += process_array(array, SIZE, iter * 100);
        
        // Modify array slightly between iterations
        for (int i = 0; i < SIZE; i += 64) {
            array[i] = array[i] + 1;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}

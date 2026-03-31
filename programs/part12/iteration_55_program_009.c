#include <stdio.h>
#include <stdint.h>

#define SIZE 1024

// Simple pseudo-random generator to create non-constant data
static inline uint32_t lcg(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

// Function containing the target loop
int process_array(volatile int *array, int n) {
    volatile int sum = 0;
    volatile int prev = 0;
    volatile int curr = 1;
    
    // Create loop-carried dependencies with distance 1
    for (int i = 0; i < n; i++) {
        // Multiple arithmetic operations to create scheduling complexity
        int val = array[i];
        
        // Recurrence 1: sum depends on previous iteration's sum
        // This creates a loop-carried dependency with distance 1
        sum = sum + val * (sum % 256);
        
        // Recurrence 2: prev/curr chain with distance 1
        // prev from iteration i is used in iteration i+1
        prev = curr;
        curr = val * 3 + prev;
        
        // Additional arithmetic to increase instruction count
        sum = (sum << 1) | (sum >> 31);  // Rotate left
        sum = sum ^ (val * 7);
        
        // Another recurrence with different operations
        prev = prev + (curr % 64);
    }
    
    return sum + prev + curr;
}

// Outer loop to provide context
void outer_loop(volatile int *array, int outer_iterations) {
    volatile int total = 0;
    
    for (int j = 0; j < outer_iterations; j++) {
        // Modify array slightly each outer iteration
        for (int i = 0; i < SIZE; i++) {
            array[i] = (array[i] * 3 + j) % 1000;
        }
        
        // Call the target inner loop
        total += process_array(array, SIZE);
    }
    
    // Ensure result is used
    printf("Final total: %d\n", total);
}

int main() {
    volatile int array[SIZE];
    uint32_t seed = 42;
    
    // Initialize array with pseudo-random values
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg(&seed) % 1000;
    }
    
    // Execute multiple times to ensure loop runs
    outer_loop(array, 10);
    
    return 0;
}

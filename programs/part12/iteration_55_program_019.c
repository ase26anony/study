#include <stdio.h>
#include <stdint.h>

#define SIZE 1024

// Simple pseudo-random generator to create non-constant data
static uint32_t lcg(uint32_t x) {
    return (1103515245 * x + 12345) & 0x7fffffff;
}

// Function containing the target loop
void process_loop(volatile int *array, int n, int *result) {
    volatile int acc1 = 0;
    volatile int acc2 = 1;
    volatile int prev = 0;
    volatile int curr = array[0];
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        acc1 = outer;
        acc2 = 1;
        prev = 0;
        curr = array[0];
        
        // Target inner loop with loop-carried dependencies
        for (int i = 0; i < n; i++) {
            // Create distance-1 dependency: prev from iteration i-1 used here
            int temp = prev * 3;
            
            // Multiple arithmetic operations to create scheduling complexity
            acc1 = acc1 + array[i] * 7;
            acc2 = acc2 * (array[i] + temp) >> 1;
            
            // Recurrence: current value depends on previous iteration
            prev = curr;
            curr = (array[i] * acc2) + (prev & 0xFF);
            
            // Additional operations to increase instruction count
            acc1 = (acc1 ^ (curr << 2)) + 1;
            acc2 = acc2 - (prev % 17);
        }
    }
    
    // Combine results to prevent elimination
    *result = acc1 + acc2 + prev + curr;
}

int main() {
    volatile int array[SIZE];
    int result = 0;
    
    // Initialize with pseudo-random values
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = lcg(seed);
        array[i] = (int)(seed % 1000);
    }
    
    // Call the function multiple times
    for (int iter = 0; iter < 5; iter++) {
        process_loop(array, SIZE, &result);
        
        // Modify array slightly between calls
        seed = lcg(seed);
        array[iter % SIZE] = (int)(seed % 1000);
    }
    
    printf("Result: %d\n", result);
    return 0;
}

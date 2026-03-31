#include <stdio.h>
#include <stdint.h>

#define SIZE 1024

// Simple pseudo-random generator to create non-constant data
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

// Function containing the target loop
int process_array(volatile int* array, int n) {
    volatile int sum = 0;
    volatile int prev = 0;
    volatile int curr = 0;
    volatile int temp1, temp2, temp3;
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        prev = 0;
        curr = array[0];
        sum = 0;
        
        // Target inner loop with loop-carried dependencies
        for (int i = 0; i < n; i++) {
            // Create distance-1 dependency chain: prev = curr from previous iteration
            prev = curr;  // This creates distance-1 use
            
            // Multiple arithmetic operations to create scheduling complexity
            temp1 = array[i] * 3;
            temp2 = temp1 + prev;  // Uses prev from previous iteration (distance-1)
            temp3 = temp2 << 2;
            
            // Accumulator with recurrence: sum depends on previous sum
            sum = sum + temp3;  // Loop-carried dependency (distance-1)
            
            // Another operation with array access
            curr = array[(i + 1) % n] + sum;  // Used as prev in next iteration
            
            // Additional operations to increase instruction count
            temp1 = temp1 ^ 0x55;
            temp2 = temp2 * 7;
        }
    }
    
    return sum;
}

int main() {
    volatile int array[SIZE];
    uint32_t seed = 42;
    
    // Initialize array with pseudo-random values
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg(&seed) % 100;
    }
    
    // Process the array multiple times
    volatile int total = 0;
    for (int iter = 0; iter < 5; iter++) {
        total += process_array(array, SIZE);
        
        // Modify array slightly each iteration
        for (int i = 0; i < SIZE; i += 64) {
            array[i] = array[i] + 1;
        }
    }
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", total);
    
    return 0;
}

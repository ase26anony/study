#include <stdio.h>
#include <stdint.h>

#define SIZE 1024

// Simple LCG for pseudo-random values
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

// Function to create loop-carried dependencies
int process_array(volatile int* array, int n) {
    volatile int sum = 0;
    volatile int prev = 0;
    volatile int curr = array[0];
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        sum = 0;
        prev = 0;
        curr = array[0];
        
        // Target inner loop with loop-carried dependencies
        for (int i = 0; i < n; i++) {
            // Multiple arithmetic operations to create scheduling complexity
            int val1 = array[i] * 3;
            int val2 = val1 >> 2;      // Shift operation
            
            // Loop-carried dependency: prev from previous iteration
            int combined = val2 + prev;
            
            // Another arithmetic operation
            int val3 = combined * 7;
            
            // Accumulator with recurrence
            sum = sum + val3;
            
            // Distance-1 dependency chain for distance1_uses
            prev = curr;               // Used in next iteration (distance=1)
            curr = array[i] + 1;       // Computed for next iteration
            
            // Additional operations to increase scheduling complexity
            int val4 = sum & 0xFF;     // Bitwise operation
            sum = sum ^ val4;          // Another operation on sum
        }
    }
    
    return sum;
}

int main() {
    // Initialize array with pseudo-random values
    volatile int array[SIZE];
    for (int i = 0; i < SIZE; i++) {
        array[i] = (int)(lcg_rand() % 100);
    }
    
    // Process the array multiple times
    volatile int total = 0;
    for (int repeat = 0; repeat < 5; repeat++) {
        total += process_array(array, SIZE);
        
        // Modify array slightly for next iteration
        for (int i = 0; i < SIZE; i += 64) {
            array[i] += 1;
        }
    }
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", total);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

// Simple pseudo-random generator to create data dependencies
static unsigned int seed = 12345;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

// Function containing the target loop
int process_array(volatile int* array, int n) {
    volatile int acc1 = 0;
    volatile int acc2 = 1;
    volatile int prev = 0;
    volatile int curr = 0;
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        prev = 0;
        curr = array[0];
        acc1 = 0;
        acc2 = 1;
        
        // Target inner loop with loop-carried dependencies
        for (int i = 0; i < n; i++) {
            // Recurrence 1: acc1 depends on its previous value (distance 1)
            int temp1 = acc1 + array[i];
            
            // Recurrence 2: acc2 depends on its previous value with arithmetic
            int temp2 = acc2 * 3 + array[i];
            
            // Distance-1 dependency chain: prev = curr from previous iteration
            int use_prev = prev * 2;  // Uses value from iteration i-1
            
            // Multiple arithmetic operations to create scheduling complexity
            int combined = (temp1 + use_prev) * 7;
            int shifted = combined >> 3;
            
            // Update recurrence variables for next iteration
            prev = curr;              // curr becomes prev for next iteration
            curr = array[i] + shifted; // New curr for next iteration
            
            // Update accumulators with cross-dependencies
            acc1 = temp1 + shifted;
            acc2 = temp2 + (acc1 & 0xFF);  // Mix with acc1
            
            // Additional operations to increase instruction count
            int extra_op = (acc1 ^ acc2) * 11;
            acc1 = acc1 + (extra_op % 256);
        }
    }
    
    // Combine results to prevent dead code elimination
    return acc1 + acc2 + prev + curr;
}

int main() {
    // Create and initialize array with volatile to prevent optimization
    volatile int array[SIZE];
    
    // Initialize with pseudo-random values
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg_rand() % 100;
    }
    
    // Process the array multiple times
    int total = 0;
    for (int iter = 0; iter < 5; iter++) {
        total += process_array(array, SIZE);
        
        // Modify array slightly between iterations
        for (int i = 0; i < SIZE; i += 17) {
            array[i] = (array[i] * 13 + 7) % 100;
        }
    }
    
    // Print result to ensure observable side effect
    printf("Result: %d\n", total);
    
    return 0;
}

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
    volatile int acc1 = 0;
    volatile int acc2 = 1;
    volatile int prev = 0;
    volatile int curr = 0;
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        acc1 = 0;
        acc2 = 1;
        prev = array[0];
        
        // Target inner loop with loop-carried dependencies
        for (int i = 0; i < n; i++) {
            // Multiple arithmetic operations to create scheduling complexity
            curr = array[i];
            
            // Distance-1 dependency: prev used in next iteration
            int diff = curr - prev;  // Uses prev from previous iteration
            
            // Recurrence 1: accumulator with multiplication
            acc1 = acc1 + diff * acc2;  // acc1 depends on previous acc1
            
            // Recurrence 2: chain of operations
            acc2 = (acc2 << 1) | (diff & 1);  // acc2 depends on previous acc2
            
            // More arithmetic to increase instruction count
            int temp = (curr * 3) + (acc1 & 0xFF);
            temp = temp ^ (acc2 >> 2);
            
            // Store result back (creates memory dependencies)
            array[i] = temp & 0x7FFFFFFF;
            
            // Setup for next iteration's distance-1 dependency
            prev = curr;
            
            // Additional operations to create more data flow
            acc1 = (acc1 * 13) + (temp & 0xF);
            acc2 = acc2 ^ (temp >> 4);
        }
    }
    
    return acc1 + acc2;
}

int main() {
    // Initialize array with volatile pseudo-random values
    volatile int array[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg_rand() % 1000;
    }
    
    // Process the array multiple times
    volatile int total_result = 0;
    
    for (int repeat = 0; repeat < 5; repeat++) {
        // Modify the array slightly each time
        for (int i = 0; i < SIZE; i++) {
            array[i] = (array[i] + repeat) & 0x7FF;
        }
        
        // Call the function with the target loop
        int result = process_array(array, SIZE);
        total_result += result;
        
        // Print intermediate result to prevent dead code elimination
        printf("Iteration %d: result = %d\n", repeat, result);
    }
    
    printf("Final total: %d\n", total_result);
    
    // Additional verification to ensure loop executed
    volatile int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array[i];
    }
    printf("Array checksum: %d\n", checksum);
    
    return 0;
}

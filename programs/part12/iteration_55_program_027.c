#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1024

// Simple LCG for generating pseudo-random values
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

// Function containing the target loop
int process_loop(volatile int* array, int n, volatile int init_val) {
    volatile int acc1 = init_val;
    volatile int acc2 = init_val * 2;
    volatile int prev = 0;
    volatile int curr = array[0];
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset accumulators for each outer iteration
        acc1 = init_val + outer;
        acc2 = init_val * 2 + outer;
        prev = 0;
        curr = array[0];
        
        // Target inner loop with loop-carried dependencies
        for (int i = 0; i < n; i++) {
            // Create distance-1 dependency chain
            prev = curr;                    // Used in next iteration
            curr = array[i];                // Defines curr for next iteration
            
            // Multiple arithmetic operations with dependencies
            int temp1 = acc1 * 3;           // Uses acc1 from previous iteration
            int temp2 = acc2 + prev;        // Uses acc2 and prev (distance-1)
            
            // More operations to create scheduling complexity
            temp1 = temp1 >> 2;             // Shift operation
            temp2 = temp2 * 7;              // Multiply operation
            
            // Update accumulators with loop-carried dependencies
            acc1 = temp1 + curr;            // Creates recurrence on acc1
            acc2 = temp2 - acc1;            // Cross-dependency between acc1 and acc2
            
            // Additional operations to increase instruction count
            int temp3 = (acc1 & 0xFF) | (acc2 & 0xFF00);
            acc1 = acc1 ^ temp3;
            acc2 = acc2 + (temp3 << 1);
        }
    }
    
    // Combine results to prevent dead code elimination
    return acc1 + acc2;
}

int main() {
    // Declare and initialize array with volatile-like behavior
    volatile int array[ARRAY_SIZE];
    
    // Fill array with pseudo-random values
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (int)(lcg_rand() % 1000);
    }
    
    // Volatile to prevent optimization of loop count
    volatile int loop_count = ARRAY_SIZE;
    
    // Execute the processing function multiple times
    int total_result = 0;
    for (int run = 0; run < 5; run++) {
        int init_val = run * 100;
        int result = process_loop(array, loop_count, init_val);
        total_result += result;
        
        // Modify array slightly between runs
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            array[i] = array[i] + 1;
        }
    }
    
    // Print result to ensure observable side effects
    printf("Final result: %d\n", total_result);
    
    return 0;
}

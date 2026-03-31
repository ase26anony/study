#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

// Simple pseudo-random generator to create data dependencies
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

// Function containing the target loop
void process_loop(volatile int *arr, int n, volatile int *result) {
    volatile int acc1 = 1;      // Accumulator with recurrence
    volatile int acc2 = 0;      // Secondary accumulator
    volatile int prev = 0;      // For distance-1 dependency
    volatile int curr = 0;
    
    // Target inner loop with loop-carried dependencies
    for (int i = 0; i < n; i++) {
        // Create distance-1 dependency: prev from iteration i-1 used here
        int temp = prev + arr[i];
        
        // Recurrence: acc1 depends on its own previous value
        // This creates loop-carried dependency across iterations
        acc1 = acc1 * 3 + temp;
        
        // Multiple arithmetic operations to increase scheduling complexity
        acc2 = (acc2 << 1) ^ (arr[i] & 0xFF);
        curr = (acc1 >> 2) + (acc2 & 0x3F);
        
        // Distance-1 chain: curr becomes prev for next iteration
        prev = curr;
        
        // More arithmetic to create data flow graph
        acc1 = acc1 + (curr * 7);
        acc2 = acc2 - (temp & 0xF);
    }
    
    // Combine results to prevent dead code elimination
    *result = acc1 + acc2 + prev;
}

// Outer loop to provide context
void outer_loop(volatile int *arr, int outer_iter, int inner_size) {
    volatile int total = 0;
    
    for (int j = 0; j < outer_iter; j++) {
        volatile int loop_result = 0;
        
        // Call the function with the target loop
        process_loop(arr + (j * 64) % SIZE, inner_size, &loop_result);
        
        // Accumulate results
        total += loop_result;
        
        // Modify array slightly for next iteration
        arr[(j * 17) % SIZE] = lcg_rand() % 1000;
    }
    
    // Print to ensure side effects are observable
    printf("Final result: %d\n", total);
}

int main(void) {
    // Initialize array with pseudo-random values
    volatile int array[SIZE];
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg_rand() % 1000;
    }
    
    // Execute multiple times to ensure loop runs
    for (int run = 0; run < 3; run++) {
        outer_loop(array, 8, 256);
        
        // Modify array between runs
        for (int i = 0; i < SIZE; i += 128) {
            array[i] = lcg_rand() % 1000;
        }
    }
    
    return 0;
}

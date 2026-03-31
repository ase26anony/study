#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

// Simple pseudo-random generator to create non-constant data
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

// Function containing the target loop
__attribute__((noinline))
static int process_array(volatile int* array, int n) {
    volatile int acc1 = 0;
    volatile int acc2 = 1;
    volatile int prev = 0;
    volatile int curr = array[0];
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        acc1 = 0;
        acc2 = 1;
        prev = 0;
        curr = array[0];
        
        // TARGET INNER LOOP - designed for modulo scheduling
        // Contains loop-carried dependencies with distance 1
        for (int i = 0; i < n; i++) {
            // Recurrence 1: acc1 depends on its previous value (distance 1)
            // This creates a true loop-carried dependency
            acc1 = acc1 + array[i] * 3;
            
            // Recurrence 2: acc2 has a more complex recurrence
            acc2 = (acc2 * 2) + (array[i] >> 1);
            
            // Distance-1 chain: prev = curr; curr = new value
            // curr from iteration i is used as prev in iteration i+1
            prev = curr;
            curr = array[i] + (prev & 0x1F);  // Use prev from previous iteration
            
            // Additional arithmetic to increase scheduling complexity
            int temp = (acc1 & 0xFF) * (acc2 & 0xFF);
            temp = temp + (array[i] << 2);
            temp = temp ^ (prev * 3);  // Uses prev from distance-1 chain
            
            // Use temp to prevent dead code elimination
            acc1 = acc1 ^ (temp & 0xFFFF);
        }
    }
    
    // Combine results to create observable output
    return (acc1 & 0xFFFF) + (acc2 & 0xFFFF) + (curr & 0xFF);
}

// Another function with different loop structure
__attribute__((noinline))
static int process_array2(volatile int* array, int n) {
    volatile int sum = 0;
    volatile int prod = 1;
    volatile int chain1 = array[0];
    volatile int chain2 = array[1];
    
    // Different loop with pointer-chase-like recurrence
    for (int i = 2; i < n; i++) {
        // Multiple interleaved recurrences
        sum = sum + array[i];
        prod = prod * (array[i] & 0x7F) + 1;  // Prevent overflow
        
        // Explicit distance-1 chain
        int old_chain1 = chain1;
        chain1 = chain2 + (old_chain1 >> 1);  // Uses old_chain1 from previous iteration
        chain2 = array[i] * 3;
        
        // More arithmetic operations
        int tmp = (sum & 0xFF) * (prod & 0xFF);
        tmp = tmp + (chain1 << 1);
        tmp = tmp - (chain2 >> 2);
        
        // Use in recurrence
        sum = sum ^ tmp;
    }
    
    return (sum & 0xFFFF) + (prod & 0xFF);
}

int main(void) {
    // Allocate and initialize array with pseudo-random values
    volatile int array[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (int)(lcg_rand() & 0xFFF);  // 12-bit values
    }
    
    // Make array size volatile to prevent constant propagation
    volatile int n = ARRAY_SIZE;
    
    // Process the array multiple times
    int result1 = 0, result2 = 0;
    for (int repeat = 0; repeat < 5; repeat++) {
        result1 += process_array(array, n);
        result2 += process_array2(array, n);
        
        // Modify array slightly between repetitions
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            array[i] = array[i] + 1;
        }
    }
    
    // Print final result to prevent dead code elimination
    printf("Result1: %d, Result2: %d\n", result1, result2);
    
    return 0;
}

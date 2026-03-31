#include <stdio.h>
#include <stdint.h>

#define SIZE 1024

// Simple pseudo-random generator to create data dependencies
static inline uint32_t lcg(uint32_t x) {
    return (1103515245u * x + 12345u) & 0x7fffffffu;
}

// Function containing the target loop
int process_array(volatile int* array, int n, int init) {
    volatile int acc1 = init;      // Volatile to prevent optimization
    volatile int acc2 = init * 2;  // Another volatile accumulator
    volatile int prev_val = 0;
    volatile int curr_val = 0;
    
    // Create loop-carried dependencies with distance 1
    for (int i = 0; i < n; i++) {
        // Multiple arithmetic operations to create scheduling complexity
        int temp1 = array[i] * 3;
        int temp2 = temp1 + acc1;      // Uses acc1 from previous iteration
        
        // Distance-1 dependency chain
        prev_val = curr_val;           // curr_val used in next iteration
        curr_val = temp2 & 0xFF;       // Current value for next iteration
        
        // More arithmetic operations
        int temp3 = (temp2 >> 4) * 7;
        int temp4 = temp3 - prev_val;  // Uses prev_val from this iteration
        
        // Recurrence: acc1 depends on its previous value
        acc1 = (acc1 * 5 + temp4) & 0xFFFF;
        
        // Another recurrence with different operations
        acc2 = (acc2 ^ temp3) + (array[i] << 2);
        
        // Additional operations to increase instruction count
        int temp5 = (i & 0xF) * 11;
        acc1 = (acc1 + temp5) & 0xFFFF;
    }
    
    // Combine results to create observable output
    return (acc1 + acc2 + curr_val) & 0xFFFF;
}

// Outer loop to provide context
void outer_loop(volatile int* array, int outer_iterations) {
    volatile int total = 0;
    
    for (int iter = 0; iter < outer_iterations; iter++) {
        // Different init values to prevent complete optimization
        int init = (iter * 17) & 0xFF;
        total += process_array(array, SIZE, init);
        
        // Modify array slightly between outer iterations
        // to prevent complete loop invariant removal
        for (int i = 0; i < 16; i++) {
            array[(iter * 16 + i) % SIZE] = (array[(iter * 16 + i) % SIZE] + 1) & 0xFF;
        }
    }
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", total);
}

int main() {
    // Initialize array with pseudo-random values
    volatile int array[SIZE];
    uint32_t seed = 42;
    
    for (int i = 0; i < SIZE; i++) {
        seed = lcg(seed);
        array[i] = (int)(seed & 0xFF);
    }
    
    // Execute the nested loop structure
    outer_loop(array, 8);
    
    return 0;
}

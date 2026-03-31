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
    volatile int acc2 = init + 1;  // Another volatile accumulator
    int temp1, temp2, temp3;
    int prev = 0;                  // For distance-1 dependency
    
    // Create loop-carried dependencies with distance 1
    for (int i = 0; i < n; i++) {
        // Distance-1 dependency: prev used in next iteration
        temp1 = prev + array[i];      // Uses prev from previous iteration
        
        // Multiple arithmetic operations to create scheduling complexity
        temp2 = temp1 * acc1;         // Uses acc1 (loop-carried)
        temp3 = (temp2 << 3) | (temp2 >> 29);  // Rotation
        
        // Another distance-1 dependency chain
        prev = temp3 & 0xFF;          // Will be used in next iteration
        
        // Loop-carried accumulator with recurrence
        acc1 = acc1 + (temp3 * array[i]);  // acc1 depends on previous acc1
        
        // More operations to increase scheduling pressure
        acc2 = acc2 ^ (temp1 + (array[i] * i));
        
        // Additional arithmetic to create more move opportunities
        array[i] = (temp1 + temp2 + temp3) & 0xFFFF;
    }
    
    // Mix results to ensure all computations are used
    return (acc1 ^ acc2) + prev;
}

// Outer loop to provide context
void outer_loop(volatile int* data, int outer_iterations) {
    volatile int outer_acc = 0;
    
    for (int iter = 0; iter < outer_iterations; iter++) {
        // Call the inner loop function multiple times
        int result = process_array(data + (iter * 64) % (SIZE - 256), 
                                  256, outer_acc);
        outer_acc = outer_acc ^ result;
        
        // Modify data slightly for next iteration
        for (int j = 0; j < 16; j++) {
            data[(iter * 16 + j) % SIZE] = lcg(data[(iter * 16 + j) % SIZE]);
        }
    }
    
    printf("Final outer accumulator: %d\n", outer_acc);
}

int main() {
    // Initialize array with pseudo-random values
    volatile int data[SIZE];
    uint32_t seed = 123456789;
    
    for (int i = 0; i < SIZE; i++) {
        seed = lcg(seed);
        data[i] = (int)(seed % 1000);
    }
    
    // Execute multiple times to ensure the loop runs
    for (int run = 0; run < 3; run++) {
        printf("Run %d:\n", run + 1);
        
        // Create a copy to prevent cross-run optimization
        volatile int local_data[SIZE];
        for (int i = 0; i < SIZE; i++) {
            local_data[i] = data[i] + run;  // Different each run
        }
        
        // Execute the outer loop
        outer_loop(local_data, 8);
        
        // Update global data
        for (int i = 0; i < SIZE; i++) {
            data[i] = local_data[i] ^ (i * run);
        }
    }
    
    // Final computation using all data
    volatile int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum = final_sum + data[i];
    }
    
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}

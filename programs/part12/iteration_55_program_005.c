#include <stdio.h>
#include <stdint.h>

#define SIZE 1024

// Simple pseudo-random generator to create non-constant data
static inline uint32_t lcg(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

// Function containing the target loop
int process_array(volatile int *arr, int n, volatile int init) {
    volatile int acc1 = init;      // First accumulator with recurrence
    volatile int acc2 = 0;         // Second accumulator
    volatile int prev = 0;         // Previous value for distance-1 dependency
    volatile int curr = 0;         // Current value
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        acc1 = init + outer;
        acc2 = 0;
        prev = 0;
        
        // TARGET INNER LOOP - designed for modulo scheduling
        for (int i = 0; i < n; i++) {
            // 1. Loop-carried dependency: acc1 depends on its previous value
            //    Creates true data dependency across iterations
            acc1 = acc1 + arr[i] * 3;
            
            // 2. Distance-1 dependency chain: prev used exactly one iteration later
            //    This should trigger distance1_uses logic
            curr = arr[i] * 2 + 7;
            acc2 = acc2 + prev * curr;  // prev from iteration i-1
            prev = curr;                // Set prev for next iteration
            
            // 3. Additional arithmetic operations to create scheduling complexity
            int temp1 = arr[i] << 2;    // Shift operation
            int temp2 = temp1 * 5;      // Multiplication
            int temp3 = temp2 + i;      // Index-based operation
            
            // 4. Another recurrence with different operations
            acc1 = acc1 ^ temp3;        // XOR operation
            
            // 5. More operations to increase instruction count
            int temp4 = (arr[i] & 0xFF) + 1;
            acc2 = acc2 - temp4;
            
            // 6. Pointer-like chase simulation
            int offset = (acc1 & 0x3F);  // Limited offset
            if (i + offset < n) {
                acc1 = acc1 + arr[i + offset];
            }
        }
    }
    
    // Combine results to prevent dead code elimination
    return acc1 + acc2;
}

int main() {
    volatile int array[SIZE];
    uint32_t seed = 42;
    
    // Initialize array with pseudo-random values
    for (int i = 0; i < SIZE; i++) {
        array[i] = (int)(lcg(&seed) % 1000);
    }
    
    // Volatile to prevent constant propagation
    volatile int iterations = SIZE;
    volatile int init_val = 17;
    
    // Process the array multiple times
    int total = 0;
    for (int repeat = 0; repeat < 5; repeat++) {
        total += process_array(array, iterations, init_val + repeat);
        
        // Modify array slightly between calls
        for (int i = 0; i < SIZE; i += 128) {
            array[i] = array[i] + 1;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}

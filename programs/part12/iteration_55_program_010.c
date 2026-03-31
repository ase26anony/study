#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

// Simple pseudo-random generator to create varying data
static unsigned int seed = 12345;
static unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

int main(void) {
    // Volatile array to prevent optimization
    volatile int array[SIZE];
    
    // Initialize array with pseudo-random values
    for (int i = 0; i < SIZE; i++) {
        array[i] = (int)lcg_rand() % 100;
    }
    
    // Outer loop to provide context
    volatile int outer_acc = 0;
    
    for (int outer = 0; outer < 10; outer++) {
        // Critical variables marked volatile to prevent optimization
        volatile int acc = 1;          // Accumulator with initial value
        volatile int prev = 0;         // Previous value for distance-1 dependency
        volatile int curr = array[0];  // Current value
        
        // Target inner loop with loop-carried dependencies
        // This is the loop we want to be modulo-scheduled
        for (int i = 0; i < SIZE; i++) {
            // Distance-1 dependency: prev from iteration i-1 used in iteration i
            int temp = prev * 3;       // Use prev from previous iteration
            
            // Recurrence (loop-carried dependency): acc depends on its previous value
            acc = acc + array[i] * 2;  // acc_i = acc_{i-1} + array[i]*2
            
            // Multiple arithmetic operations to create scheduling complexity
            curr = array[i] * 5 + (i & 7);  // Current value computation
            int shifted = curr << 2;        // Shift operation
            int multiplied = shifted * 3;   // Multiplication
            
            // Mix operations to create dataflow graph
            acc = acc + multiplied / 4;     // More accumulation
            
            // Update prev for next iteration's distance-1 use
            prev = curr & 0xFF;             // Store for use in next iteration
            
            // Additional operations to increase instruction count
            int extra = (acc & 1) ? temp : multiplied;
            acc = acc ^ (extra << 1);
        }
        
        // Use the result to prevent dead code elimination
        outer_acc += acc;
        
        // Modify array slightly for each outer iteration
        // to prevent complete loop invariant code motion
        for (int i = 0; i < SIZE; i += 64) {
            array[i] = array[i] + outer;
        }
    }
    
    // Print result to ensure observable side effect
    printf("Result: %d\n", outer_acc);
    
    return 0;
}

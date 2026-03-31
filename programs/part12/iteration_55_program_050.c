#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

// Simple pseudo-random generator to create non-constant data
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
        array[i] = (int)lcg_rand();
    }
    
    // Outer loop to provide context
    volatile int outer_acc = 0;
    
    for (int outer = 0; outer < 10; outer++) {
        // Critical variables marked volatile to prevent optimization
        volatile int acc = outer_acc;  // Loop-carried dependency
        volatile int prev = 0;
        volatile int curr = array[0];
        volatile int* volatile ptr = (volatile int*)array;  // Volatile pointer
        
        // Target inner loop with loop-carried dependencies
        // This is the loop we want to be modulo-scheduled
        for (int i = 0; i < SIZE; i++) {
            // Recurrence 1: accumulator with loop-carried dependency (distance 1)
            // acc_i depends on acc_{i-1}
            int temp = acc;
            acc = acc + array[i] * 3;
            
            // Recurrence 2: prev/curr chain (distance 1 use)
            // prev_i = curr_{i-1}
            prev = curr;
            curr = array[i] + prev;  // curr_i depends on prev_i (which is curr_{i-1})
            
            // Multiple arithmetic operations to create scheduling complexity
            int mult = array[i] * 7;
            int shift = mult >> 2;
            int add = shift + 13;
            
            // Pointer chase with volatile to create memory dependencies
            int val = *ptr;
            ptr = (volatile int*)((char*)ptr + sizeof(int));
            
            // Mix operations to create dataflow graph
            acc = acc - (val & 0xFF);
            acc = acc + (add % 17);
            
            // Another recurrence: dependent chain across iterations
            static volatile int chain = 0;
            chain = chain * 3 + array[i];
            acc = acc ^ (chain & 1);
        }
        
        // Use results to prevent dead code elimination
        outer_acc = outer_acc + (acc & 0x7FFF);
        outer_acc = outer_acc ^ (prev & 0xFF);
        outer_acc = outer_acc * 3 - (curr % 256);
    }
    
    // Print result to ensure observable side effect
    printf("Result: %d\n", outer_acc);
    
    return 0;
}

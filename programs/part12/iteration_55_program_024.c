#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

// Simple pseudo-random generator to create data dependencies
static unsigned int seed = 12345;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

// Function containing the target loop
int process_array(volatile int* array, int n) {
    volatile int acc1 = 0;
    volatile int acc2 = 1;
    volatile int prev = 0;
    volatile int curr = array[0];
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        acc1 = outer;
        acc2 = 1;
        prev = 0;
        curr = array[0];
        
        // Target inner loop with loop-carried dependencies
        // This should trigger modulo scheduling
        for (int i = 0; i < n; i++) {
            // Create distance-1 dependency: prev from iteration i-1 used here
            int temp = prev * 3;  // Uses prev from previous iteration
            
            // Multiple arithmetic operations to create scheduling complexity
            acc1 = acc1 + array[i] * acc2;  // Recurrence: acc1 depends on previous acc1
            acc2 = acc2 * 2 + (array[i] & 0x1F);  // Another recurrence
            
            // Pointer-chase like dependency chain
            prev = curr;          // prev gets current value
            curr = array[i] + temp;  // curr uses temp which used prev
            
            // More operations to increase instruction count
            acc1 = (acc1 << 1) | (acc1 >> 31);  // Rotate
            acc2 = acc2 ^ (array[i] * 7);
            
            // Array access with stride to prevent optimization
            int idx = (i * 13) % n;
            volatile int val = array[idx];
            acc1 = acc1 - val;
        }
    }
    
    // Combine results to prevent dead code elimination
    return acc1 + acc2 + prev + curr;
}

int main() {
    // Allocate and initialize array with volatile to prevent optimization
    volatile int* array = (volatile int*)malloc(SIZE * sizeof(int));
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with pseudo-random values
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg_rand() % 100;
    }
    
    // Process the array multiple times
    int total = 0;
    for (int repeat = 0; repeat < 5; repeat++) {
        total += process_array(array, SIZE);
        
        // Modify array slightly between calls
        for (int i = 0; i < SIZE; i += 17) {
            array[i] = (array[i] * 3 + 1) % 100;
        }
    }
    
    // Print result to ensure observable side effect
    printf("Result: %d\n", total);
    
    free((void*)array);
    return 0;
}

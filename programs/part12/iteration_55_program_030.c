#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

// Simple pseudo-random generator to create data dependencies
static unsigned int seed = 12345;
static unsigned int lcg_rand(void) {
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
        acc1 = 0;
        acc2 = 1;
        prev = 0;
        curr = array[0];
        
        // TARGET INNER LOOP - designed for modulo scheduling
        // Contains loop-carried dependencies with distance 1
        for (int i = 0; i < n; i++) {
            // Recurrence 1: acc1 depends on its previous value (distance 1)
            acc1 = acc1 + array[i] * 3;
            
            // Recurrence 2: acc2 depends on its previous value (distance 1)
            acc2 = (acc2 * 2) + (array[i] & 0x1F);
            
            // Distance-1 chain: prev = curr from previous iteration
            int temp = prev * 7;      // Use prev from iteration i-1
            prev = curr;              // curr becomes prev for next iteration
            curr = array[i] + temp;   // New curr using old prev
            
            // Additional arithmetic to create complex dataflow
            acc1 = acc1 ^ (curr << 2);
            acc2 = acc2 - (prev >> 1);
            
            // More operations to increase scheduling complexity
            int t1 = acc1 * acc2;
            int t2 = t1 + (array[i] % 17);
            acc1 = t2 & 0xFFFF;
            acc2 = (t2 >> 16) + (prev & 0xFF);
        }
    }
    
    // Combine results to prevent dead code elimination
    return (acc1 + acc2 + prev + curr) & 0x7FFFFFFF;
}

int main(void) {
    // Allocate and initialize array with volatile-like behavior
    volatile int* array = (volatile int*)malloc(SIZE * sizeof(int));
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with pseudo-random values
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg_rand() % 1000;
    }
    
    // Process the array multiple times
    int total = 0;
    for (int iter = 0; iter < 5; iter++) {
        // Vary the loop bound slightly to prevent excessive unrolling
        int bound = SIZE - (iter % 8);
        total += process_array(array, bound);
    }
    
    // Print result to prevent optimization
    printf("Result: %d\n", total);
    
    free((void*)array);
    return 0;
}

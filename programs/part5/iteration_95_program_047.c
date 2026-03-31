#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

// Prevent inlining to ensure the loop structure is preserved
__attribute__((noinline)) 
long modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  // Prevent outer loop unrolling
    long total_sum = 0;
    
    // Outer loop with volatile control
    while (outer_counter < OUTER_ITERATIONS) {
        long sum = 0;
        int prev_val = a[0];  // Create loop-carried dependency
        
        // Inner loop with complex dependencies
        for (int i = 0; i < size; i++) {
            // Multiple loads with potential aliasing
            int a_val = a[i];
            int b_val = b[i];
            
            // Operation with non-unit latency (multiplication)
            int product = a_val * b_val;
            
            // Loop-carried dependency: sum depends on previous iteration
            sum = sum + product;
            
            // Distance-1 dependency: a[i] depends on a[i-1]
            // This creates distance1_uses scenario
            int temp = prev_val + b_val;
            
            // Mix of operations with different latencies
            temp = temp & 0xFFFF;      // Bitwise operation
            temp = temp * 3;           // Another multiplication
            temp = temp | 0x1000;      // Another bitwise op
            
            // Store with loop-carried dependency
            a[i] = temp;
            prev_val = temp;           // Update for next iteration
            
            // Additional computation to create more scheduling pressure
            int extra = (a_val ^ b_val) & 0xFF;
            sum = sum - (extra * 2);   // Mixed add/sub operations
            
            // Another distance-0 dependency within same iteration
            int check = (sum & 1) ? product : extra;
            a[i] = a[i] + check;       // Update again
        }
        
        total_sum += sum;
        outer_counter++;
        
        // Add some control flow variability
        if (rand() % 2) {
            // Additional operation to vary control flow
            total_sum = total_sum ^ 0xABCD;
        }
    }
    
    return total_sum;
}

int main() {
    srand(time(NULL));
    
    // Allocate and initialize arrays
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with pseudo-random data
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    // Call the loop function
    long result = modulo_scheduled_loop(a, b, SIZE);
    
    // Print result to prevent dead code elimination
    printf("Result: %ld\n", result);
    
    // Verify computation (optional)
    long verify = 0;
    for (int i = 0; i < SIZE; i++) {
        verify += a[i] + b[i];
    }
    printf("Verification sum: %ld\n", verify);
    
    free(a);
    free(b);
    
    return 0;
}

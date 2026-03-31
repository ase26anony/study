#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

// Force no inlining to preserve loop structure
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  // Prevent optimization
    int total_sum = 0;
    
    // Outer loop with volatile control
    for (outer_counter = 0; outer_counter < OUTER_ITERATIONS; outer_counter++) {
        int sum = 0;
        int prev = a[0];  // For loop-carried dependency
        
        // Inner loop with multiple dependencies
        for (int i = 0; i < size; i++) {
            // Load operations (potential distance-0 dependencies)
            int a_val = a[i];
            int b_val = b[i];
            
            // Create distance-1 dependency: use previous iteration's value
            int temp = prev;
            
            // Mix of operations with different latencies
            // Multiplication has higher latency than addition
            int product = a_val * b_val;  // Multi-cycle operation
            
            // Multiple uses of same value (creates distance1_uses scenarios)
            int sum_update = sum + product;
            int alt_use = product & 0xFF;  // Additional use
            
            // Loop-carried dependency (distance-1)
            int new_val = temp + b_val;
            
            // Update with bitwise operations (different latency profile)
            new_val = new_val ^ alt_use;
            new_val = new_val | (product >> 8);
            
            // Store back with loop-carried dependency
            a[i] = new_val;
            prev = new_val;  // For next iteration
            
            // Accumulate with loop-carried dependency
            sum = sum_update;
            
            // Additional arithmetic to create more scheduling opportunities
            int extra = (a_val & b_val) | (a_val ^ b_val);
            sum += (extra & 0x1);  // Small addition
        }
        
        total_sum += sum;
        
        // Conditional branch based on random to create control variability
        if (rand() % 2) {
            // Small additional computation to affect scheduling
            total_sum ^= 1;
        }
    }
    
    return total_sum;
}

int main() {
    // Initialize with random data
    srand(time(NULL));
    
    int a[SIZE];
    int b[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    // Call the loop function
    int result = modulo_scheduled_loop(a, b, SIZE);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Also print a few array values to ensure they're used
    printf("Sample a[0]: %d, a[100]: %d\n", a[0], a[100]);
    
    return 0;
}

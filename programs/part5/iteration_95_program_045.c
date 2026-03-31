#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

// Use volatile to prevent excessive optimization
volatile int outer_counter = OUTER_ITERATIONS;

// Mark as noinline to ensure function isn't inlined
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int n) {
    volatile int control = 0;
    int sum = 0;
    int i;
    
    // Outer loop with volatile control
    while (outer_counter-- > 0) {
        // Add some control flow variability
        if (rand() % 2) {
            control = 1;
        } else {
            control = 0;
        }
        
        // Critical inner loop with multiple dependencies
        // This creates both distance-0 and distance-1 dependencies
        sum = 0;
        
        // Initialize first element with dependency chain
        a[0] = b[0] * control;
        
        // Main computational loop
        for (i = 1; i < n; i++) {
            // Multiple operations with different latencies
            int product = a[i] * b[i];           // Multi-cycle multiply
            int shifted = product << 2;          // Fast shift
            int masked = shifted & 0xFF;         // Bitwise operation
            
            // Loop-carried dependency (distance-1)
            // a[i] depends on a[i-1] from previous iteration
            int prev_val = a[i-1];
            int delta = prev_val + masked;       // Integer add
            
            // Multiple uses of the same value (creates distance1_uses)
            int temp1 = delta * 3;               // Another multiply
            int temp2 = delta + 7;               // Another add
            a[i] = (temp1 + temp2) & 0xFFFF;     // Combined operation
            
            // Accumulator with loop-carried dependency (distance-1)
            sum = sum + product;                 // Critical accumulation
            
            // Additional operations to create complex scheduling graph
            int extra = (a[i] ^ b[i]) | (product & 0xF);
            b[i] = extra + i;
        }
        
        // Post-loop processing to prevent tail optimization
        if (control) {
            sum = sum * 2;
        }
    }
    
    return sum;
}

int main() {
    int a[SIZE];
    int b[SIZE];
    int i, result;
    
    // Seed random number generator
    srand(time(NULL));
    
    // Initialize arrays with pseudo-random data
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    // Reset outer counter
    outer_counter = OUTER_ITERATIONS;
    
    // Call the function with the loop structure
    result = modulo_scheduled_loop(a, b, SIZE);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Also print some array values to ensure they're used
    printf("Sample values: a[100]=%d, b[100]=%d\n", a[100], b[100]);
    
    return 0;
}

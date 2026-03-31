#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

// Force the compiler to keep the function separate
__attribute__((noinline))
static int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  // Prevent outer loop unrolling
    int total_sum = 0;
    
    // Outer loop with volatile control
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        int prev = a[0];  // Create loop-carried dependency
        
        // Inner loop with complex dependencies
        for (int i = 1; i < size - 1; i++) {
            // Multiple loads with potential aliasing
            int a_val = a[i];
            int b_val = b[i];
            
            // Operations with different latencies
            int product = a_val * b_val;      // Multi-cycle multiply
            int shifted = product >> 3;       // Fast operation
            int masked = shifted & 0xFF;      // Bitwise operation
            
            // Loop-carried dependency on sum (distance-1)
            sum = sum + masked;               // Add with carry dependency
            
            // Another loop-carried dependency on array (distance-1)
            int temp = prev + b_val;          // Use previous iteration's value
            prev = a_val;                     // Update for next iteration
            
            // Multiple uses of the same value within iteration
            int combined = (temp & 0x7F) | (masked & 0x80);
            a[i] = combined + (sum & 1);      // Write back with dependency
            
            // Additional arithmetic to create more scheduling opportunities
            int extra = (a_val ^ b_val) * (i & 0xF);
            sum = sum - (extra & 0x3F);       // Another dependency on sum
            
            // Create distance-0 dependencies within same iteration
            int check = (a_val > b_val) ? a_val : b_val;
            b[i] = check + (combined & 0xF);
        }
        
        // Use volatile to prevent optimization
        total_sum += sum;
        outer_counter++;
        
        // Add some conditional control flow in outer loop
        if (rand() % 2) {
            a[0] = (a[0] + 1) & 0xFF;
        }
    }
    
    return total_sum;
}

int main(void) {
    // Initialize with random data
    srand(time(NULL));
    
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Fill arrays with pseudo-random data
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    // Run the modulo-scheduled loop
    int result = modulo_scheduled_loop(a, b, SIZE);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Verify some computation
    int verify = 0;
    for (int i = 0; i < 10; i++) {
        verify += a[i] + b[i];
    }
    printf("Verification sum: %d\n", verify);
    
    free(a);
    free(b);
    
    return 0;
}

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
        // Create control flow variability
        if (rand() % 2) {
            control = 1;
        } else {
            control = 0;
        }
        
        // Critical inner loop with cross-iteration dependencies
        // This should trigger modulo scheduling
        for (i = 1; i < n; i++) {
            // Load operations with potential cache effects
            int a_val = a[i];
            int b_val = b[i];
            int a_prev = a[i-1];
            
            // Multiple operations with different latencies
            // Integer multiply has higher latency than add
            int product = a_val * b_val;  // Higher latency operation
            
            // Loop-carried dependency: sum depends on previous iteration
            sum = sum + product;  // distance-1 dependency
            
            // Another loop-carried dependency with distance-1
            // a[i] depends on a[i-1] from previous iteration
            int temp = a_prev + b_val;
            
            // Multiple uses of the same computed value
            // This creates distance1_uses scenarios
            int temp2 = temp & 0xFF;      // Bitwise operation
            int temp3 = temp2 | 0x80;     // Another bitwise operation
            int temp4 = temp3 ^ 0x55;     // XOR operation
            
            // Store with potential anti-dependency
            a[i] = temp4 + control;  // distance-1 dependency
            
            // Additional arithmetic to create more scheduling opportunities
            int extra = (sum & 0x7F) * 3;  // Multiply with mask
            b[i] = b[i-1] + extra;        // Another distance-1 dependency
            
            // Complex expression with mixed operations
            // This creates varied instruction latencies
            sum = sum + ((a_val & b_val) | (a_prev ^ product));
        }
        
        // Boundary case for i=0
        if (n > 0) {
            a[0] = (a[0] * b[0]) & 0xFF;
            sum += a[0];
        }
    }
    
    return sum;
}

// Another complex loop with different pattern
__attribute__((noinline))
int secondary_loop(int *a, int *b, int n) {
    int sum = 0;
    volatile int mod = 7;
    
    for (int i = 2; i < n; i++) {
        // Chain of dependencies
        int x = a[i] * mod;           // Multiply
        int y = x + a[i-1];           // Add with distance-1
        int z = y - b[i-2];           // Subtract with distance-2
        int w = z & b[i];             // Bitwise AND
        
        // Multiple uses of w
        a[i] = w + (w >> 2);          // Use w twice
        b[i] = (b[i-1] * w) & 0xFF;   // Multiply with distance-1
        
        // Accumulate with complex expression
        sum += (x * y) + (z | w);     // Mixed operations
        
        // Additional dependency chain
        mod = (mod * 13 + 1) & 0x3F;  // Update volatile
    }
    
    return sum;
}

int main() {
    // Initialize with different seed each run
    srand(time(NULL));
    
    // Allocate and initialize arrays
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
    
    // Reset volatile counter
    outer_counter = OUTER_ITERATIONS;
    
    // Call the function that should trigger modulo scheduling
    int result1 = modulo_scheduled_loop(a, b, SIZE);
    
    // Call secondary loop to provide more scheduling opportunities
    int result2 = secondary_loop(a, b, SIZE);
    
    // Use results to prevent dead code elimination
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Final sum: %d\n", result1 + result2);
    
    // Clean up
    free(a);
    free(b);
    
    return 0;
}

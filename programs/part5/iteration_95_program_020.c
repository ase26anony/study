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
    
    // Outer loop with volatile control to prevent unrolling
    while (outer_counter-- > 0) {
        // Add some conditional branching based on rand()
        if (rand() % 2) {
            control = 1;
        } else {
            control = 0;
        }
        
        // Critical inner loop with multiple dependencies
        // This should trigger modulo scheduling
        for (i = 0; i < n; i++) {
            // Load operations with potential cache effects
            int a_val = a[i];
            int b_val = b[i];
            
            // Multiple arithmetic operations with different latencies
            // Integer multiply has higher latency than add
            int product = a_val * b_val;  // Higher latency operation
            
            // Loop-carried dependency: sum depends on previous iteration
            sum = sum + product;  // distance-1 dependency
            
            // Another loop-carried dependency with distance-1
            // a[i] depends on a[i-1] from previous iteration
            if (i > 0) {
                // Multiple uses of the same computed value
                int temp = a[i-1] + b_val;  // distance-1 use
                a[i] = (temp & 0xFF) | (product ^ 0xAA);  // Mixed operations
                
                // Additional use of temp to create distance1_uses scenario
                sum = sum ^ (temp >> 2);  // Another distance-1 use
            } else {
                // Boundary case
                a[i] = product + b_val;
            }
            
            // More operations to create complex scheduling graph
            int diff = a_val - b_val;
            sum = sum + (diff & 0x3F);  // Bitwise operation
            
            // Cross-iteration dependency through array b
            if (i < n - 1) {
                b[i+1] = b[i+1] + (sum & 1);  // Another distance-1 dependency
            }
        }
        
        // Additional computation between inner loop iterations
        if (control) {
            sum = sum * 3;  // Different latency operation
        }
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
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    // Reset volatile counter
    outer_counter = OUTER_ITERATIONS;
    
    // Call the loop function
    int result = modulo_scheduled_loop(a, b, SIZE);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Cleanup
    free(a);
    free(b);
    
    return 0;
}

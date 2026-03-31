#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure - marked to prevent optimizations
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iterations) {
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Declare many local variables to increase register pressure
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        // Complex expression that will be reused - candidate for rematerialization
        // This creates a pattern where recomputing might be cheaper than spilling
        int complex_expr = (a * b) + (c << 2) - (d & 0xFF);
        
        // Force data dependencies with volatile inputs
        v1 = a + i;
        v2 = b - i;
        v3 = c * i;
        v4 = d ^ i;
        
        // Use the complex expression multiple times with different operations
        // This creates register copies that might be replaced with recomputation
        v5 = complex_expr + v1;
        v6 = complex_expr - v2;
        v7 = complex_expr * v3;
        v8 = complex_expr ^ v4;
        
        // More independent calculations to increase register pressure
        v9 = (v1 * v2) + (v3 << 1);
        v10 = (v2 * v3) - (v4 >> 1);
        v11 = (v3 * v4) ^ (v1 & 0xF);
        v12 = (v4 * v1) | (v2 << 3);
        
        // Compiler barrier - prevents reordering/coalescing
        __asm__ volatile ("" : : : "memory");
        
        // More calculations using the complex expression again
        v13 = complex_expr + v9;
        v14 = complex_expr - v10;
        v15 = complex_expr * v11;
        v16 = complex_expr ^ v12;
        
        // Additional independent expressions
        v17 = v5 * v6 + v7;
        v18 = v6 * v7 - v8;
        v19 = v7 * v8 ^ v5;
        v20 = v8 * v5 | v6;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Conditional branch to split basic blocks
        // Uses volatile condition to prevent optimization
        volatile int condition = (i & 1);
        if (condition) {
            // Branch 1: More calculations
            v21 = v9 + v10 * v11;
            v22 = v10 - v11 * v12;
            v23 = v11 ^ v12 * v9;
            v24 = v12 | v9 * v10;
            
            // Reuse complex expression in this branch
            v25 = complex_expr + v21;
            v26 = complex_expr - v22;
            v27 = complex_expr * v23;
            v28 = complex_expr ^ v24;
            
            // More operations to maintain pressure
            v29 = v21 * v22 + v23;
            v30 = v22 * v23 - v24;
        } else {
            // Branch 2: Different calculations
            v21 = v13 * v14 + v15;
            v22 = v14 * v15 - v16;
            v23 = v15 ^ v16 * v13;
            v24 = v16 | v13 * v14;
            
            // Reuse complex expression in this branch too
            v25 = complex_expr + v21;
            v26 = complex_expr - v22;
            v27 = complex_expr * v23;
            v28 = complex_expr ^ v24;
            
            // Different operations in this branch
            v29 = v21 + v22 * v23;
            v30 = v22 - v23 * v24;
        }
        
        // Merge point - all variables are live here
        // This creates high register pressure at the merge
        result += v25 + v26 + v27 + v28 + v29 + v30;
        
        // Final compiler barrier
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Initialize volatile variables with random values
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 100;  // Enough iterations to create pressure
    
    printf("Starting computation with high register pressure...\n");
    printf("Parameters: a=%d, b=%d, c=%d, d=%d, iterations=%d\n", 
           a, b, c, d, iterations);
    
    // Call the high-pressure function
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

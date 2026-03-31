#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure and rematerialization patterns
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                         volatile int c, volatile int d,
                                         volatile int e, volatile int f) {
    // Many local variables to create register pressure
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    // Complex expression that will be reused (rematerialization candidate)
    int complex_expr;
    
    // Initialize with volatile inputs to prevent constant folding
    v1 = a + b;
    v2 = c * d;
    v3 = e ^ f;
    v4 = a - b;
    v5 = c + d;
    v6 = e * f;
    v7 = a ^ c;
    v8 = b ^ d;
    v9 = e + a;
    v10 = f + b;
    
    // Memory barrier to prevent reordering
    __asm__ volatile ("" : : : "memory");
    
    // Create the complex expression that will be rematerialized
    // This is a non-trivial computation that will be used multiple times
    complex_expr = (v1 * v2) + (v3 << 2) - (v4 / (v5 + 1)) ^ (v6 & 0xFF);
    
    // Force multiple uses of the complex expression through copies
    // This creates register-to-register moves that are candidates for rematerialization
    v11 = complex_expr;
    v12 = complex_expr;
    v13 = complex_expr;
    
    __asm__ volatile ("" : : : "memory");
    
    // More computations to increase register pressure
    v14 = v11 + v1;
    v15 = v12 * v2;
    v16 = v13 ^ v3;
    v17 = v14 - v4;
    v18 = v15 + v5;
    v19 = v16 * v6;
    v20 = v17 ^ v7;
    
    __asm__ volatile ("" : : : "memory");
    
    // Additional copies of the complex expression
    v21 = complex_expr;
    v22 = complex_expr;
    v23 = complex_expr;
    
    // More arithmetic operations
    v24 = v21 + v8;
    v25 = v22 * v9;
    v26 = v23 ^ v10;
    v27 = v24 - v11;
    v28 = v25 + v12;
    v29 = v26 * v13;
    v30 = v27 ^ v14;
    
    __asm__ volatile ("" : : : "memory");
    
    // Loop with conditional to split basic blocks
    volatile int iterations = 5;
    volatile int condition = a & 1;
    
    int sum = 0;
    for (volatile int i = 0; i < iterations; i++) {
        // Recompute complex_expr inside loop (another rematerialization opportunity)
        int loop_expr = (v1 * v2) + (v3 << 2) - (v4 / (v5 + 1)) ^ (v6 & 0xFF);
        
        // Conditional branch to split live ranges
        if (condition) {
            // Use loop_expr in true branch
            v15 = loop_expr + v16;
            v17 = loop_expr * v18;
            v19 = loop_expr ^ v20;
            __asm__ volatile ("" : : : "memory");
        } else {
            // Use loop_expr in false branch  
            v21 = loop_expr - v22;
            v23 = loop_expr + v24;
            v25 = loop_expr * v26;
            __asm__ volatile ("" : : : "memory");
        }
        
        // More computations that keep many values live
        v27 = v15 + v17;
        v28 = v19 * v21;
        v29 = v23 ^ v25;
        v30 = v27 + v28;
        
        // Accumulate to prevent dead code elimination
        sum += v30 + loop_expr;
        
        // Modify condition to create varying control flow
        condition = (condition + i) & 1;
        __asm__ volatile ("" : : : "memory");
    }
    
    // Final computation using all variables
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                 sum + complex_expr;
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Use volatile inputs to prevent constant propagation
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int e = rand() % 100 + 1;
    volatile int f = rand() % 100 + 1;
    
    // Call the high-pressure function
    volatile int result = high_pressure_compute(a, b, c, d, e, f);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}

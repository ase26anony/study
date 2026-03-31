#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Prevent inlining and interprocedural optimization
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iterations) {
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Declare many local variables to create register pressure
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        // Complex expression that will be reused - rematerialization candidate
        int complex_expr = (a * b) + (c << 2) - d;
        
        // First sequence of independent computations
        v1 = a + b;
        v2 = b * c;
        v3 = c - d;
        v4 = d << 1;
        v5 = a ^ b;
        v6 = b | c;
        v7 = c & d;
        v8 = ~a;
        v9 = a * 3;
        v10 = b / 2;
        
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // Reuse the complex expression multiple times
        v11 = complex_expr + v1;
        v12 = complex_expr - v2;
        v13 = complex_expr * v3;
        v14 = complex_expr & v4;
        v15 = complex_expr | v5;
        
        // More independent computations
        v16 = v1 * v2 + v3;
        v17 = v4 - v5 * v6;
        v18 = (v7 << 3) + v8;
        v19 = v9 ^ v10;
        v20 = v11 | v12;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Control flow split based on volatile condition
        volatile int condition = a & 1;
        if (condition) {
            // Branch with more computations
            v21 = v13 + v14 * v15;
            v22 = v16 - v17 / 2;
            v23 = v18 << (v19 & 3);
            v24 = v20 ^ v21;
            v25 = v22 | v23;
            
            // Recompute the complex expression again
            v26 = complex_expr + v24;
            v27 = complex_expr - v25;
            
            // More arithmetic
            v28 = v26 * v27 + a;
            v29 = v28 - b * c;
            v30 = v29 ^ d;
            
            result += v30;
        } else {
            // Alternative branch with different computations
            v21 = v13 - v14;
            v22 = v15 * v16;
            v23 = v17 >> 1;
            v24 = v18 ^ v19;
            v25 = v20 | v21;
            
            // Another copy of the complex expression
            v26 = complex_expr * v22;
            v27 = complex_expr & v23;
            
            // More operations
            v28 = v24 + v25 * 2;
            v29 = v26 - v27;
            v30 = v28 ^ v29;
            
            result -= v30;
        }
        
        // Final computations mixing all variables
        int mix1 = v1 + v6 + v11 + v16 + v21 + v26;
        int mix2 = v2 - v7 + v12 - v17 + v22 - v27;
        int mix3 = v3 * v8 * v13 * v18 * v23 * v28;
        int mix4 = v4 & v9 & v14 & v19 & v24 & v29;
        int mix5 = v5 | v10 | v15 | v20 | v25 | v30;
        
        // Use the complex expression one more time
        int final_expr = complex_expr + mix1 - mix2 + mix3 - mix4 + mix5;
        
        // Update result with final computation
        result ^= final_expr;
        
        // Modify inputs slightly for next iteration
        a = (a + 1) & 0xFF;
        b = (b * 3) & 0xFF;
        c = (c - 1) & 0xFF;
        d = (d ^ i) & 0xFF;
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Initialize volatile inputs with random values
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 100;
    
    // Call the high-pressure computation function
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure - marked to prevent optimizations
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                          volatile int c, volatile int d,
                                          volatile int iterations) {
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Declare many local variables to create register pressure
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        // Complex expression that will be reused - candidate for rematerialization
        int complex_expr = (a * b) + (c << 2) - (d & 0xFF);
        
        // First set of independent computations
        v1 = a + b + i;
        v2 = b * c - i;
        v3 = c ^ d ^ i;
        v4 = d << (i & 3);
        v5 = (a & b) | (c & d);
        
        // Use the complex expression multiple times
        v6 = complex_expr + v1;
        v7 = complex_expr - v2;
        v8 = complex_expr * v3;
        
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // More computations
        v9 = v1 * v2 + v3;
        v10 = v2 / (v4 + 1) + v5;
        v11 = v3 ^ v4 ^ v5;
        v12 = v4 << 2;
        v13 = v5 >> 1;
        
        // Use complex expression again
        v14 = complex_expr & v6;
        v15 = complex_expr | v7;
        v16 = complex_expr ^ v8;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Conditional branch to split basic blocks
        if (a & 1) {
            // Branch 1 computations
            v17 = v9 + v10 + v11;
            v18 = v12 * v13 - v14;
            v19 = v15 ^ v16 ^ complex_expr;
            v20 = v17 << (v18 & 3);
            
            // More uses of complex expression
            v21 = complex_expr + v17;
            v22 = complex_expr - v18;
            v23 = complex_expr * v19;
        } else {
            // Branch 2 computations
            v17 = v9 - v10 + v11;
            v18 = v12 / (v13 + 1) - v14;
            v19 = v15 | v16 | complex_expr;
            v20 = v17 >> (v18 & 3);
            
            // More uses of complex expression
            v21 = complex_expr & v17;
            v22 = complex_expr | v18;
            v23 = complex_expr ^ v19;
        }
        
        // Compiler barrier after conditional
        __asm__ volatile ("" : : : "memory");
        
        // Final computations merging both branches
        v24 = v20 + v21 + v22;
        v25 = v23 * complex_expr;
        v26 = v24 ^ v25;
        v27 = v26 << 2;
        v28 = v27 - complex_expr;
        v29 = v28 & 0xFFFF;
        v30 = v29 | (complex_expr & 0xFF);
        
        // Accumulate result
        result += v30;
        
        // Modify inputs slightly for next iteration
        a = (a + 1) & 0xFF;
        b = (b - 1) & 0xFF;
        c = (c ^ i) & 0xFF;
        d = (d + i) & 0xFF;
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Initialize volatile inputs with random values
    volatile int a = rand() % 256;
    volatile int b = rand() % 256;
    volatile int c = rand() % 256;
    volatile int d = rand() % 256;
    volatile int iterations = 100 + (rand() % 100);
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    // Call the high-pressure function
    volatile int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

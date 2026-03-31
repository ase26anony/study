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
        // This pattern: (a * b) + (c << 2) appears multiple times
        int complex_expr = (a * b) + (c << 2);
        
        // First group of independent calculations
        v1 = complex_expr + d;
        v2 = complex_expr - d;
        v3 = complex_expr * d;
        v4 = complex_expr ^ d;
        v5 = (complex_expr & d) | (a ^ b);
        
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // More calculations using the same complex expression
        v6 = complex_expr + i;
        v7 = complex_expr - i;
        v8 = complex_expr * i;
        v9 = complex_expr ^ i;
        v10 = (complex_expr & i) | (c ^ d);
        
        // Another complex expression that will be reused
        int complex_expr2 = (b * c) + (d << 3);
        
        // More independent calculations
        v11 = complex_expr2 + a;
        v12 = complex_expr2 - a;
        v13 = complex_expr2 * a;
        v14 = complex_expr2 ^ a;
        v15 = (complex_expr2 & a) | (b ^ c);
        
        // Compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Use both complex expressions together
        v16 = complex_expr + complex_expr2;
        v17 = complex_expr - complex_expr2;
        v18 = complex_expr * complex_expr2;
        v19 = complex_expr ^ complex_expr2;
        v20 = (complex_expr & complex_expr2) | (a ^ d);
        
        // Conditional branch to split basic blocks
        volatile int condition = a & 1;
        if (condition) {
            // Branch 1: more calculations
            v21 = complex_expr << 1;
            v22 = complex_expr >> 1;
            v23 = complex_expr2 << 2;
            v24 = complex_expr2 >> 2;
            v25 = (v21 * v22) + (v23 << 1);
            
            // Recompute complex_expr (forcing potential rematerialization)
            int complex_expr_copy = (a * b) + (c << 2);
            v26 = complex_expr_copy + v25;
            v27 = complex_expr_copy - v25;
            
            // Compiler barrier
            __asm__ volatile ("" : : : "memory");
        } else {
            // Branch 2: alternative calculations
            v21 = complex_expr * 3;
            v22 = complex_expr / 2;
            v23 = complex_expr2 * 5;
            v24 = complex_expr2 / 3;
            v25 = (v21 + v22) * (v23 - v24);
            
            // Another copy of the complex expression
            int complex_expr_copy2 = (a * b) + (c << 2);
            v26 = complex_expr_copy2 ^ v25;
            v27 = complex_expr_copy2 | v25;
            
            // Compiler barrier
            __asm__ volatile ("" : : : "memory");
        }
        
        // Merge point - use values from both branches
        v28 = v21 + v22 + v23 + v24;
        v29 = v25 * v26 * v27;
        v30 = (v28 ^ v29) & complex_expr;
        
        // Final accumulation with compiler barrier
        __asm__ volatile ("" : : : "memory");
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
        
        // Modify inputs slightly for next iteration
        a ^= i;
        b += i;
        c -= i;
        d |= i;
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Use volatile inputs to prevent constant propagation
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 10; // Small number to avoid long execution
    
    printf("Starting computation with: a=%d, b=%d, c=%d, d=%d, iter=%d\n",
           a, b, c, d, iterations);
    
    volatile int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

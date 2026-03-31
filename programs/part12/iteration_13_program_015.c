#include <stdio.h>
#include <stdlib.h>

// Function with high register pressure - marked to prevent optimizations
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
        int v31, v32, v33, v34, v35, v36, v37, v38, v39, v40;
        
        // Complex expression that will be reused - candidate for rematerialization
        int complex_expr = (a * b) + (c << 2) - (d / 3);
        
        // First group of independent calculations
        v1 = a + b + complex_expr;
        v2 = b * c - complex_expr;
        v3 = c ^ d + complex_expr;
        v4 = d & a - complex_expr;
        v5 = (a << 3) | b + complex_expr;
        
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // Second group with the same complex expression
        v6 = complex_expr * 2;
        v7 = complex_expr + v1;
        v8 = complex_expr - v2;
        v9 = complex_expr ^ v3;
        v10 = complex_expr & v4;
        
        // More independent calculations
        v11 = (v1 * v2) + (v3 << 1);
        v12 = (v2 / v3) | (v4 ^ 0xFF);
        v13 = (v3 + v4) * (v5 - 1);
        v14 = (v4 << 2) + (v5 >> 1);
        v15 = (v5 & 0x7F) | (v1 << 8);
        
        __asm__ volatile ("" : : : "memory");
        
        // Third group reusing complex_expr
        v16 = complex_expr + v11;
        v17 = complex_expr * v12;
        v18 = complex_expr - v13;
        v19 = complex_expr ^ v14;
        v20 = complex_expr & v15;
        
        // More arithmetic to increase pressure
        v21 = v6 + v7 * v8;
        v22 = v9 - v10 / v11;
        v23 = v12 ^ v13 & v14;
        v24 = v15 | v16 << v17;
        v25 = v18 + v19 * v20;
        
        __asm__ volatile ("" : : : "memory");
        
        // Fourth group with complex_expr
        v26 = complex_expr + v21;
        v27 = complex_expr * v22;
        v28 = complex_expr - v23;
        v29 = complex_expr ^ v24;
        v30 = complex_expr & v25;
        
        // Final calculations
        v31 = v26 * v27 + v28;
        v32 = v29 - v30 ^ v21;
        v33 = v22 | v23 & v24;
        v34 = v25 << 2 + v26;
        v35 = v27 / (v28 + 1);
        
        // Control flow split based on volatile condition
        volatile int condition = a & 1;
        if (condition) {
            // Branch 1 calculations
            v36 = v31 + v32 * v33;
            v37 = v34 - v35 / v26;
            v38 = v27 ^ v28 & v29;
            v39 = v30 | v31 << 1;
            v40 = complex_expr * 3;  // Reuse again in branch
            
            result += v36 + v37 + v38 + v39 + v40;
        } else {
            // Branch 2 calculations
            v36 = v32 - v31 * v34;
            v37 = v33 + v35 / v27;
            v38 = v28 ^ v29 & v30;
            v39 = v31 | v32 << 2;
            v40 = complex_expr / 2;  // Reuse again in branch
            
            result += v36 * v37 - v38 + v39 - v40;
        }
        
        // Force another reuse of complex_expr after the branch
        int final_calc = complex_expr + v33 + v34 + v35;
        result += final_calc;
        
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

int main(void) {
    // Initialize with random values to prevent constant propagation
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 10;  // Small loop to keep runtime reasonable
    
    printf("Starting computation with: a=%d, b=%d, c=%d, d=%d, iter=%d\n",
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

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
        
        // Complex expression that will be reused - potential rematerialization candidate
        int complex_expr = (a * b) + (c << 2) - d;
        
        // First sequence of independent calculations
        v1 = a + b;
        v2 = b * c;
        v3 = c - d;
        v4 = d << 1;
        v5 = a ^ b;
        v6 = b | c;
        v7 = c & d;
        v8 = ~a;
        v9 = a * 3;
        v10 = b * 5;
        
        // Use the complex expression multiple times with different operations
        v11 = complex_expr + v1;
        v12 = complex_expr - v2;
        v13 = complex_expr * v3;
        v14 = complex_expr & v4;
        v15 = complex_expr | v5;
        
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // More calculations
        v16 = v1 * v2;
        v17 = v3 + v4;
        v18 = v5 ^ v6;
        v19 = v7 << 2;
        v20 = v8 >> 1;
        
        // Use complex_expr again
        v21 = complex_expr + v16;
        v22 = complex_expr - v17;
        v23 = complex_expr * v18;
        
        // Conditional branch to split basic blocks
        volatile int condition = a > b;
        if (condition) {
            // Different calculations in the true branch
            v24 = v9 * v10;
            v25 = v11 + v12;
            v26 = v13 - v14;
            v27 = v15 ^ v16;
            v28 = v17 | v18;
            
            // Use complex_expr in the branch
            v29 = complex_expr + v24;
            v30 = complex_expr - v25;
            
            result += v29 + v30;
        } else {
            // Different calculations in the false branch
            v24 = v10 / (a != 0 ? a : 1);
            v25 = v11 * v12;
            v26 = v13 + v14;
            v27 = v15 & v16;
            v28 = v17 ^ v18;
            
            // Use complex_expr in this branch too
            v29 = complex_expr * v24;
            v30 = complex_expr | v25;
            
            result += v26 + v27 + v28;
        }
        
        // More calculations after the conditional
        int v31 = v19 + v20;
        int v32 = v21 - v22;
        int v33 = v23 * v24;
        int v34 = v25 ^ v26;
        int v35 = v27 | v28;
        
        // Use complex_expr one more time
        int v36 = complex_expr + v31;
        int v37 = complex_expr - v32;
        int v38 = complex_expr * v33;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Final calculations mixing everything
        int v39 = v29 + v30 + v31;
        int v40 = v32 * v33 / (v34 != 0 ? v34 : 1);
        int v41 = v35 ^ v36;
        int v42 = v37 | v38;
        int v43 = v39 - v40;
        int v44 = v41 * v42;
        int v45 = (v43 << 3) + v44;
        
        // Update result with all values to prevent dead code elimination
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                  v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39 + v40 +
                  v41 + v42 + v43 + v44 + v45;
        
        // Modify inputs slightly for next iteration
        a = (a + 1) & 0xFF;
        b = (b - 1) & 0xFF;
        c = (c * 2) & 0xFF;
        d = (d >> 1) & 0xFF;
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
    volatile int iterations = 10; // Small number to avoid long execution
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Prevent inlining and interprocedural optimization
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                          volatile int c, volatile int d,
                                          volatile int iterations) {
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Declare many local variables to create high register pressure
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        int v31, v32, v33, v34, v35, v36, v37, v38, v39, v40;
        
        // Complex expression that will be reused - potential rematerialization candidate
        int complex_expr = (a * b) + (c << 2) - (d / 3);
        
        // Force data dependencies and prevent optimization
        v1 = a + i;
        v2 = b - i;
        v3 = c * i;
        v4 = d ^ i;
        
        // Use the complex expression multiple times with different operations
        v5 = complex_expr + v1;
        v6 = complex_expr - v2;
        v7 = complex_expr * v3;
        v8 = complex_expr ^ v4;
        
        // More independent calculations
        v9 = (v1 * v2) + (v3 << 1);
        v10 = (v2 * v3) - (v4 >> 1);
        v11 = (v3 * v4) ^ (v1 & 0xFF);
        v12 = (v4 * v1) | (v2 & 0x0F);
        
        // Compiler barrier - prevent reordering/coalescing
        __asm__ volatile ("" : : : "memory");
        
        // Another use of the complex expression
        v13 = complex_expr + v9;
        v14 = complex_expr - v10;
        
        // More arithmetic to increase register pressure
        v15 = v5 * v6 + v7;
        v16 = v6 * v7 - v8;
        v17 = v7 * v8 ^ v5;
        v18 = v8 * v5 | v6;
        v19 = v9 * v10 + v11;
        v20 = v10 * v11 - v12;
        
        // Control flow split based on volatile condition
        volatile int condition = (i & 1);
        if (condition) {
            // Different computations in this branch
            v21 = v13 * v14 + v15;
            v22 = v14 * v15 - v16;
            v23 = v15 * v16 ^ v17;
            v24 = v16 * v17 | v18;
            
            // Reuse complex expression again
            v25 = complex_expr + v21;
            v26 = complex_expr - v22;
            
            v27 = v19 * v20 + v21;
            v28 = v20 * v21 - v22;
            v29 = v21 * v22 ^ v23;
            v30 = v22 * v23 | v24;
        } else {
            // Alternative computations in else branch
            v21 = v15 * v16 + v17;
            v22 = v16 * v17 - v18;
            v23 = v17 * v18 ^ v19;
            v24 = v18 * v19 | v20;
            
            // Another reuse of complex expression
            v25 = complex_expr * v21;
            v26 = complex_expr ^ v22;
            
            v27 = v13 * v14 + v15;
            v28 = v14 * v15 - v16;
            v29 = v15 * v16 ^ v17;
            v30 = v16 * v17 | v18;
        }
        
        // Compiler barrier between basic blocks
        __asm__ volatile ("" : : : "memory");
        
        // More calculations after the conditional
        v31 = v25 * v26 + v27;
        v32 = v26 * v27 - v28;
        v33 = v27 * v28 ^ v29;
        v34 = v28 * v29 | v30;
        
        // Final reuse of complex expression
        v35 = complex_expr + v31;
        v36 = complex_expr - v32;
        v37 = complex_expr * v33;
        v38 = complex_expr ^ v34;
        
        v39 = v31 * v32 + v33;
        v40 = v32 * v33 - v34;
        
        // Compiler barrier before result accumulation
        __asm__ volatile ("" : : : "memory");
        
        // Accumulate results to prevent dead code elimination
        result += v35 + v36 + v37 + v38 + v39 + v40;
        
        // Additional compiler barrier in loop
        __asm__ volatile ("" : : : "memory");
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
    volatile int iterations = 100; // Enough iterations to create pressure
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    volatile int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

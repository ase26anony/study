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
        int complex_expr = (a * b) + (c << 2) - (d / 3);
        
        // First sequence of independent calculations
        v1 = a + b + i;
        v2 = b * c - i;
        v3 = c ^ d ^ i;
        v4 = d << (i & 3);
        v5 = complex_expr + v1;  // Use complex expression
        v6 = v1 * v2 + v3;
        v7 = v2 | v3 | v4;
        v8 = v3 - v4 + v5;
        v9 = v4 ^ v5 ^ v6;
        v10 = v5 + v6 + v7;
        
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // Second sequence with more calculations
        v11 = v6 * v7 - v8;
        v12 = v7 | v8 | v9;
        v13 = v8 - v9 + v10;
        v14 = v9 ^ v10 ^ v11;
        v15 = complex_expr + v11;  // Reuse same complex expression
        v16 = v10 + v11 + v12;
        v17 = v11 * v12 + v13;
        v18 = v12 | v13 | v14;
        v19 = v13 - v14 + v15;
        v20 = v14 ^ v15 ^ v16;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Third sequence
        v21 = v15 + v16 + v17;
        v22 = v16 * v17 - v18;
        v23 = v17 | v18 | v19;
        v24 = v18 - v19 + v20;
        v25 = complex_expr + v21;  // Reuse again
        v26 = v19 ^ v20 ^ v21;
        v27 = v20 + v21 + v22;
        v28 = v21 * v22 + v23;
        v29 = v22 | v23 | v24;
        v30 = v23 - v24 + v25;
        
        // Control flow split to create multiple basic blocks
        volatile int condition = (i & 1);
        if (condition) {
            // Branch 1: More calculations
            v1 = v25 + v26;
            v2 = v26 * v27;
            v3 = v27 | v28;
            v4 = complex_expr + v28;  // Reuse in branch
            v5 = v28 - v29 + v30;
            result += v1 + v2 + v3 + v4 + v5;
        } else {
            // Branch 2: Different calculations
            v6 = v25 * v26;
            v7 = v26 + v27;
            v8 = v27 ^ v28;
            v9 = complex_expr + v29;  // Reuse in other branch
            v10 = v28 | v29 | v30;
            result += v6 + v7 + v8 + v9 + v10;
        }
        
        // Final barrier before loop continues
        __asm__ volatile ("" : : : "memory");
        
        // Force all variables to be used to prevent dead code elimination
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Initialize volatile inputs to prevent constant propagation
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 10;  // Small iteration count to avoid overflow
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    volatile int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

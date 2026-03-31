#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure - marked to prevent optimizations
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                          volatile int c, volatile int d) {
    // Many local variables to create register pressure
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    // Complex expression that will be reused - candidate for rematerialization
    int complex_expr;
    
    // Volatile iteration count to prevent loop unrolling
    volatile int iterations = 10;
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // Compute the complex expression (will be used multiple times)
        complex_expr = (a * b) + (c << 2) - (d / 3);
        
        // First set of independent calculations using the complex expression
        v1 = complex_expr + a;
        v2 = complex_expr - b;
        v3 = complex_expr * c;
        v4 = complex_expr / (d + 1);
        v5 = complex_expr ^ a;
        
        // Compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // More calculations creating many live values
        v6 = v1 * v2 + v3;
        v7 = v2 - v4 * v5;
        v8 = v3 ^ v1 | v2;
        v9 = v4 + v5 * v6;
        v10 = v5 - v6 / v7;
        
        // Reuse the complex expression again
        v11 = complex_expr + v1;
        v12 = complex_expr - v2;
        v13 = complex_expr * v3;
        
        // Compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Conditional branch to split basic blocks
        if (a & 1) {
            // More calculations in the true branch
            v14 = v6 * v7 + v8;
            v15 = v7 - v8 * v9;
            v16 = v8 ^ v9 | v10;
            v17 = v9 + v10 * v11;
            v18 = v10 - v11 / v12;
            
            // Reuse complex expression again
            v19 = complex_expr + v14;
            v20 = complex_expr - v15;
        } else {
            // Different calculations in the false branch
            v14 = v6 + v7 * v8;
            v15 = v7 - v8 / v9;
            v16 = v8 | v9 ^ v10;
            v17 = v9 * v10 + v11;
            v18 = v10 / v11 - v12;
            
            // Reuse complex expression
            v19 = complex_expr * v14;
            v20 = complex_expr / v15;
        }
        
        // Compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // More calculations after the branch
        v21 = v11 + v12 * v13;
        v22 = v12 - v13 / v14;
        v23 = v13 ^ v14 | v15;
        v24 = v14 + v15 * v16;
        v25 = v15 - v16 / v17;
        
        // Reuse complex expression yet again
        v26 = complex_expr + v21;
        v27 = complex_expr - v22;
        v28 = complex_expr * v23;
        
        // Compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Final set of calculations
        v29 = v16 + v17 * v18;
        v30 = v17 - v18 / v19;
        
        // Accumulate results to prevent dead code elimination
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
        
        // Modify inputs slightly to prevent constant propagation
        a ^= 1;
        b += i;
        c -= i;
        d ^= i;
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Volatile inputs to prevent constant propagation
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    
    // Call the high-pressure function
    volatile int result = high_pressure_compute(a, b, c, d);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}

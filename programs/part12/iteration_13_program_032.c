#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure that should trigger rematerialization
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int e, volatile int f) {
    volatile int iterations = 10;
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Declare many local variables to create register pressure
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        // Complex expression that will be reused - potential rematerialization candidate
        int common_expr = (a * b) + (c << 2) - (d & 0xFF) + (e | 0x7F);
        
        // Force many independent computations to keep variables live
        v1 = a + b + common_expr;
        v2 = b * c - common_expr;
        v3 = c ^ d + common_expr;
        v4 = d | e - common_expr;
        v5 = e & f + common_expr;
        
        // Compiler barrier to prevent reordering/coalescing
        __asm__ volatile ("" : : : "memory");
        
        // More computations using the same common expression
        v6 = common_expr * 2;
        v7 = common_expr / 3;
        v8 = common_expr ^ 0xABCD;
        v9 = common_expr | 0x1234;
        v10 = common_expr & 0x5678;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Additional complex expressions that might create more pseudo registers
        v11 = (v1 * v2) + (v3 << 1);
        v12 = (v2 / v3) | (v4 >> 2);
        v13 = (v3 ^ v4) & (v5 << 3);
        v14 = (v4 | v5) + (v6 >> 1);
        v15 = (v5 & v6) - (v7 << 2);
        
        // Control flow split based on volatile condition
        volatile int condition = (i & 1);
        if (condition) {
            // Different computations in this branch
            v16 = v11 * v12 + common_expr;
            v17 = v12 / v13 - common_expr;
            v18 = v13 ^ v14 + common_expr;
            v19 = v14 | v15 - common_expr;
            v20 = v15 & v16 + common_expr;
            
            // Another instance of the common expression
            v21 = common_expr * 3;
            v22 = common_expr ^ 0xDEAD;
        } else {
            // Alternative computations
            v16 = v11 + v12 - common_expr;
            v17 = v12 - v13 * common_expr;
            v18 = v13 + v14 / common_expr;
            v19 = v14 - v15 | common_expr;
            v20 = v15 + v16 & common_expr;
            
            // Yet another instance
            v21 = common_expr / 2;
            v22 = common_expr | 0xBEEF;
        }
        
        // More computations after the branch
        v23 = v16 * v17 + v18;
        v24 = v17 / v18 - v19;
        v25 = v18 ^ v19 + v20;
        v26 = v19 | v20 - v21;
        v27 = v20 & v21 + v22;
        
        // Final compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Final computations mixing everything
        v28 = (v23 << 2) + (v24 >> 1);
        v29 = (v25 & 0xFF) | (v26 ^ 0xAA);
        v30 = (v27 * 7) - (v28 / 3);
        
        // Accumulate result to prevent dead code elimination
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
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
    volatile int e = rand() % 100 + 1;
    volatile int f = rand() % 100 + 1;
    
    // Call the high-pressure function
    int result = high_pressure_compute(a, b, c, d, e, f);
    
    // Print result to prevent optimization
    printf("Result: %d\n", result);
    
    return 0;
}

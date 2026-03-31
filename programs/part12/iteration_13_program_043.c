#include <stdio.h>
#include <stdlib.h>

// Prevent inlining and interprocedural optimization
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                          volatile int c, volatile int d,
                                          volatile int e, volatile int f) {
    // Force many pseudo registers with independent calculations
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    volatile int v31, v32, v33, v34, v35, v36, v37, v38, v39, v40;
    
    // Complex expression that will be reused - candidate for rematerialization
    volatile int common_expr;
    
    // Loop with volatile iteration count to create multiple basic blocks
    volatile int iterations = 10;
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // Create the common sub-expression
        // This complex expression should be a good rematerialization candidate
        common_expr = (a * b) + (c << 2) - (d / (e + 1)) ^ f;
        
        // Force many independent calculations using the common expression
        // Each assignment creates a new pseudo register
        v1 = common_expr + a;
        v2 = common_expr - b;
        v3 = common_expr * c;
        v4 = common_expr / (d + 1);
        v5 = common_expr ^ e;
        v6 = common_expr | f;
        v7 = common_expr & a;
        v8 = common_expr << 2;
        v9 = common_expr >> 1;
        v10 = common_expr + b * 3;
        
        // More calculations to increase register pressure
        v11 = v1 * v2 + v3;
        v12 = v4 - v5 * v6;
        v13 = v7 ^ v8 | v9;
        v14 = v10 << (v1 & 3);
        v15 = v11 >> (v2 % 4);
        v16 = v12 * v13 - v14;
        v17 = v15 + v16 / (v3 + 1);
        v18 = v17 ^ v1 & v2;
        v19 = v18 | v3 ^ v4;
        v20 = v19 << (v5 % 8);
        
        // Reuse the common expression again - this copy might be replaced by recomputation
        v21 = common_expr + v6;
        v22 = common_expr - v7;
        v23 = common_expr * v8;
        v24 = common_expr / (v9 + 1);
        v25 = common_expr ^ v10;
        
        // Even more calculations
        v26 = v20 + v21 * v22;
        v27 = v23 - v24 ^ v25;
        v28 = v26 | v27 & v21;
        v29 = v28 << (v22 % 4);
        v30 = v29 >> 1;
        
        // Conditional branch to split basic blocks
        if (a & 1) {  // Volatile condition
            // Another compiler barrier
            __asm__ volatile ("" : : : "memory");
            
            // Different calculations in this branch
            v31 = v30 * 3 + v1;
            v32 = v2 - v31 / 2;
            v33 = v3 ^ v32 | v4;
            v34 = v5 << (v33 & 3);
            v35 = v6 >> 1;
            
            // Reuse common expression in the branch
            v36 = common_expr + v31;
            v37 = common_expr - v32;
            
            result += v31 + v32 + v33 + v34 + v35 + v36 + v37;
        } else {
            // Another compiler barrier
            __asm__ volatile ("" : : : "memory");
            
            // Alternative calculations
            v38 = v30 / 2 + v7;
            v39 = v8 - v38 * 3;
            v40 = v9 ^ v39 & v10;
            
            // Reuse common expression again
            int temp1 = common_expr + v38;
            int temp2 = common_expr - v39;
            
            result += v38 + v39 + v40 + temp1 + temp2;
        }
        
        // Final compiler barrier in loop
        __asm__ volatile ("" : : : "memory");
        
        // Use all variables to prevent dead code elimination
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        result += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    }
    
    return result;
}

int main() {
    // Initialize with random values to prevent constant propagation
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

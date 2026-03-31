#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Prevent inlining and interprocedural optimization
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                          volatile int c, volatile int d,
                                          volatile int iter_count) {
    volatile int result = 0;
    
    for (volatile int i = 0; i < iter_count; i++) {
        // Declare many local variables to create high register pressure
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        int v31, v32, v33, v34, v35, v36, v37, v38, v39, v40;
        
        // Complex expression that will be reused - rematerialization candidate
        int common_expr = (a * b) + (c << 2) - (d / 3);
        
        // Force many independent computations to create register pressure
        v1 = a + b + i;
        v2 = b * c - i;
        v3 = c ^ d ^ i;
        v4 = d << (i & 3);
        v5 = common_expr + v1;  // First use of common_expr
        
        __asm__ volatile ("" : : : "memory");  // Compiler barrier
        
        v6 = v1 * v2 + v3;
        v7 = v2 | v3 | v4;
        v8 = v3 - v4 + v5;
        v9 = v4 ^ v5 ^ v1;
        v10 = common_expr * v2;  // Second use of common_expr
        
        v11 = v5 + v6 + v7;
        v12 = v6 * v7 - v8;
        v13 = v7 ^ v8 ^ v9;
        v14 = v8 << (v9 & 3);
        v15 = common_expr - v3;  // Third use of common_expr
        
        __asm__ volatile ("" : : : "memory");  // Compiler barrier
        
        v16 = v9 + v10 + v11;
        v17 = v10 * v11 - v12;
        v18 = v11 ^ v12 ^ v13;
        v19 = v12 << (v13 & 3);
        v20 = common_expr | v4;  // Fourth use of common_expr
        
        v21 = v13 + v14 + v15;
        v22 = v14 * v15 - v16;
        v23 = v15 ^ v16 ^ v17;
        v24 = v16 << (v17 & 3);
        v25 = common_expr + v5;  // Fifth use of common_expr
        
        __asm__ volatile ("" : : : "memory");  // Compiler barrier
        
        v26 = v17 + v18 + v19;
        v27 = v18 * v19 - v20;
        v28 = v19 ^ v20 ^ v21;
        v29 = v20 << (v21 & 3);
        v30 = common_expr * v6;  // Sixth use of common_expr
        
        // Create control flow to split basic blocks
        volatile int condition = a ^ b ^ c ^ d ^ i;
        if (condition & 1) {
            // Additional computations in the true branch
            v31 = v21 + v22 + v23;
            v32 = v22 * v23 - v24;
            v33 = v23 ^ v24 ^ v25;
            v34 = v24 << (v25 & 3);
            v35 = common_expr - v7;  // Seventh use of common_expr
            
            __asm__ volatile ("" : : : "memory");  // Compiler barrier
            
            v36 = v25 + v26 + v27;
            v37 = v26 * v27 - v28;
            v38 = v27 ^ v28 ^ v29;
            v39 = v28 << (v29 & 3);
            v40 = common_expr | v8;  // Eighth use of common_expr
            
            // More complex expressions
            v31 = v31 * v32 + v33;
            v32 = v32 | v33 | v34;
            v33 = v33 - v34 + v35;
            v34 = v34 ^ v35 ^ v36;
            v35 = common_expr + v9;  // Ninth use of common_expr
        } else {
            // Different computations in the false branch
            v31 = v21 * v22 + v23;
            v32 = v22 | v23 | v24;
            v33 = v23 - v24 + v25;
            v34 = v24 ^ v25 ^ v26;
            v35 = common_expr * v10;  // Tenth use of common_expr
            
            __asm__ volatile ("" : : : "memory");  // Compiler barrier
            
            v36 = v25 + v26 + v27;
            v37 = v26 * v27 - v28;
            v38 = v27 ^ v28 ^ v29;
            v39 = v28 << (v29 & 3);
            v40 = common_expr - v11;  // Eleventh use of common_expr
        }
        
        // Merge point - use many variables to keep them live
        int temp_sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                      v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                      v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                      v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39 + v40;
        
        result += temp_sum & 0xFF;  // Prevent overflow
        result ^= common_expr;  // Twelfth use of common_expr
        
        __asm__ volatile ("" : : : "memory");  // Compiler barrier
    }
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    // Use volatile inputs to prevent constant propagation
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iter_count = 10;  // Small iteration count to avoid overflow
    
    volatile int result = high_pressure_compute(a, b, c, d, iter_count);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}

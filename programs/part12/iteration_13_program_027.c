#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure that should trigger early rematerialization
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, volatile int c, 
                                 volatile int d, volatile int e, volatile int f) {
    // Many local variables to create register pressure
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    // Complex expression that will be reused - good candidate for rematerialization
    int common_expr = (a * b) + (c << 2) - (d & 0xFF);
    
    // Initial computations to create many live values
    v1 = a + b;
    v2 = b * c;
    v3 = c - d;
    v4 = d ^ e;
    v5 = e | f;
    
    __asm__ volatile ("" : : : "memory"); // Prevent reordering
    
    v6 = v1 * v2;
    v7 = v2 + v3;
    v8 = v3 ^ v4;
    v9 = v4 | v5;
    v10 = v5 & v1;
    
    // Reuse the common expression - this creates register copies
    v11 = common_expr;
    v12 = common_expr + v1;
    
    __asm__ volatile ("" : : : "memory");
    
    v13 = v6 * v7;
    v14 = v7 + v8;
    v15 = v8 ^ v9;
    v16 = v9 | v10;
    v17 = v10 & v6;
    
    // Another reuse of the common expression
    v18 = common_expr - v2;
    v19 = common_expr * v3;
    
    volatile int loop_cond = 100;
    volatile int branch_cond = a > 0;
    
    // Loop to extend live ranges and create control flow
    for (volatile int i = 0; i < loop_cond; i++) {
        __asm__ volatile ("" : : : "memory");
        
        // More computations inside loop
        v20 = v13 + v14;
        v21 = v14 * v15;
        v22 = v15 ^ v16;
        v23 = v16 | v17;
        v24 = v17 & v13;
        
        // Conditional branch to split basic blocks
        if (branch_cond) {
            // Reuse common expression in one branch
            v25 = common_expr + v20;
            v26 = common_expr - v21;
            
            v27 = v20 * v21;
            v28 = v21 + v22;
            v29 = v22 ^ v23;
            v30 = v23 | v24;
        } else {
            // Different computations in other branch
            v25 = v20 - v21;
            v26 = v21 * v22;
            
            v27 = v22 + v23;
            v28 = v23 ^ v24;
            v29 = v24 & v20;
            v30 = v20 | v21;
        }
        
        __asm__ volatile ("" : : : "memory");
        
        // More computations merging both paths
        v1 = v25 + v26;
        v2 = v26 * v27;
        v3 = v27 ^ v28;
        v4 = v28 | v29;
        v5 = v29 & v30;
        
        // Final reuse of common expression
        v6 = common_expr + v1;
        v7 = common_expr - v2;
        
        // Prevent loop unrolling
        __asm__ volatile ("" : : : "memory");
    }
    
    // Combine results to return
    volatile int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
                         v8 + v9 + v10 + v11 + v12 + v13 + v14 + 
                         v15 + v16 + v17 + v18 + v19 + v20;
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    // Use volatile inputs to prevent constant propagation
    volatile int a = rand() % 100;
    volatile int b = rand() % 100;
    volatile int c = rand() % 100;
    volatile int d = rand() % 100;
    volatile int e = rand() % 100;
    volatile int f = rand() % 100;
    
    // Call the high-pressure function
    int result = high_pressure_compute(a, b, c, d, e, f);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}

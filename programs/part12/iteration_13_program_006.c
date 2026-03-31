#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Prevent inlining and interprocedural optimization
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int e, volatile int f)
{
    volatile int iterations = (a % 10) + 15; // Force loop iterations
    volatile int result = 0;
    
    // Create many local variables to increase register pressure
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    int v31, v32, v33, v34, v35, v36, v37, v38, v39, v40;
    
    // Complex expression that will be reused - candidate for rematerialization
    int complex_expr;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // Force data dependencies with volatile inputs
        v1 = a + i;
        v2 = b * i;
        v3 = c ^ i;
        v4 = d - i;
        v5 = e + (i << 2);
        v6 = f * (i + 1);
        
        // Create the complex expression that will be reused
        // This is a prime candidate for rematerialization
        complex_expr = (v1 * v2) + (v3 << 2) - (v4 / (v5 + 1)) + (v6 ^ 0xABCD);
        
        // Use the complex expression multiple times with different operations
        // Creating many pseudo registers that may need rematerialization
        v7 = complex_expr + v1;
        v8 = complex_expr - v2;
        v9 = complex_expr * v3;
        v10 = complex_expr ^ v4;
        v11 = complex_expr & v5;
        v12 = complex_expr | v6;
        
        // More independent calculations to increase register pressure
        v13 = (v1 + v2) * (v3 - v4);
        v14 = (v5 ^ v6) + (v7 << 1);
        v15 = (v8 * v9) - (v10 / 2);
        v16 = (v11 & 0xFF) | (v12 << 8);
        v17 = v13 + v14 + v15 + v16;
        v18 = v1 * v3 * v5 * v7;
        v19 = v2 + v4 + v6 + v8;
        v20 = (v9 << 3) ^ (v10 >> 2);
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Reuse complex_expr again with different variables
        // This creates more opportunities for register copies
        v21 = complex_expr + v13;
        v22 = complex_expr - v14;
        v23 = complex_expr * v15;
        v24 = complex_expr ^ v16;
        v25 = complex_expr & v17;
        v26 = complex_expr | v18;
        
        // More arithmetic to keep values live
        v27 = v19 * v20 + v21;
        v28 = v22 - v23 * v24;
        v29 = (v25 << 4) | (v26 >> 4);
        v30 = v27 ^ v28 ^ v29;
        
        // Conditional branch to split basic blocks
        volatile int condition = a + b + c;
        if (condition > 1000) {
            // Different computation path
            v31 = complex_expr * 2;  // Another use of complex_expr
            v32 = v1 + v3 + v5 + v7 + v9;
            v33 = v2 * v4 * v6 * v8 * v10;
            v34 = (v11 << 1) | (v12 >> 1);
            v35 = v31 + v32 - v33 + v34;
            
            // More variables in this branch
            v36 = v35 * complex_expr;  // Yet another use
            v37 = v36 ^ 0x1234;
            v38 = v37 & 0xFFFF;
            v39 = v38 << 2;
            v40 = v39 >> 1;
            
            result += v40;
        } else {
            // Alternative path with different computations
            v31 = complex_expr / 2;  // Reuse in else branch
            v32 = v1 - v3 + v5 - v7 + v9;
            v33 = v2 + v4 + v6 + v8 + v10;
            v34 = (v11 >> 1) | (v12 << 1);
            v35 = v31 * v32 + v33 - v34;
            
            v36 = v35 ^ complex_expr;  // Reuse again
            v37 = v36 & 0xABCD;
            v38 = v37 | 0x4321;
            v39 = v38 * 3;
            v40 = v39 - 100;
            
            result += v40;
        }
        
        // Final barrier to prevent optimization across iterations
        __asm__ volatile ("" : : : "memory");
        
        // Mix all results to prevent dead code elimination
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        result += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
        result += v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39 + v40;
    }
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    // Use volatile inputs to prevent constant propagation
    volatile int a = rand();
    volatile int b = rand();
    volatile int c = rand();
    volatile int d = rand();
    volatile int e = rand();
    volatile int f = rand();
    
    printf("Inputs: a=%d, b=%d, c=%d, d=%d, e=%d, f=%d\n", 
           a, b, c, d, e, f);
    
    // Call the high-pressure function
    int result = high_pressure_compute(a, b, c, d, e, f);
    
    printf("Result: %d\n", result);
    
    return 0;
}

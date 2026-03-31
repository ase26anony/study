#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure that should trigger early rematerialization
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, volatile int c, 
                                 volatile int d, volatile int e, volatile int f) {
    // Declare many local variables to create register pressure
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    // Complex expression that will be reused - candidate for rematerialization
    int complex_expr;
    
    // Use volatile loop counter to prevent optimization
    volatile int iterations = 10;
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // Complex expression computed once but used multiple times
        // This creates a pattern where rematerialization might be cheaper than spilling
        complex_expr = (a * b) + (c << 2) - (d / (e + 1)) + (f ^ 0x55AA);
        
        // Force many independent computations to create register pressure
        v1 = complex_expr + a;
        v2 = complex_expr + b;
        v3 = complex_expr + c;
        v4 = complex_expr + d;
        v5 = complex_expr + e;
        
        // More computations using the same complex expression
        v6 = complex_expr * 2;
        v7 = complex_expr * 3;
        v8 = complex_expr * 4;
        v9 = complex_expr * 5;
        v10 = complex_expr * 6;
        
        // Compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Additional independent computations
        v11 = (a + b) * (c - d);
        v12 = (b + c) * (d - e);
        v13 = (c + d) * (e - f);
        v14 = (d + e) * (f - a);
        v15 = (e + f) * (a - b);
        
        v16 = v11 ^ v12;
        v17 = v12 ^ v13;
        v18 = v13 ^ v14;
        v19 = v14 ^ v15;
        v20 = v15 ^ v11;
        
        // Compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // More computations with different patterns
        v21 = (v1 << 3) | (v2 >> 1);
        v22 = (v3 << 2) | (v4 >> 2);
        v23 = (v5 << 1) | (v6 >> 3);
        v24 = (v7 << 4) | (v8 >> 4);
        v25 = (v9 << 5) | (v10 >> 5);
        
        v26 = v21 + v22 + v23;
        v27 = v24 + v25 + v21;
        v28 = v22 + v23 + v24;
        v29 = v25 + v21 + v22;
        v30 = v23 + v24 + v25;
        
        // Conditional branch to split basic blocks
        volatile int condition = a & 1;
        if (condition) {
            // Different computation path
            v1 = v1 ^ 0x1234;
            v2 = v2 ^ 0x5678;
            v3 = v3 ^ 0x9ABC;
            v4 = v4 ^ 0xDEF0;
            v5 = v5 ^ 0x1357;
            
            // Recompute complex_expr in this branch
            complex_expr = (b * c) + (d << 2) - (e / (f + 1)) + (a ^ 0xAA55);
            
            v6 = complex_expr + b;
            v7 = complex_expr + c;
            v8 = complex_expr + d;
            v9 = complex_expr + e;
            v10 = complex_expr + f;
        } else {
            // Alternative path with different computations
            v11 = v11 | 0xF0F0;
            v12 = v12 | 0x0F0F;
            v13 = v13 | 0xAAAA;
            v14 = v14 | 0x5555;
            v15 = v15 | 0xCCCC;
            
            // Another instance of similar complex expression
            int complex_expr2 = (c * d) + (e << 2) - (f / (a + 1)) + (b ^ 0x33CC);
            
            v16 = complex_expr2 * 7;
            v17 = complex_expr2 * 8;
            v18 = complex_expr2 * 9;
            v19 = complex_expr2 * 10;
            v20 = complex_expr2 * 11;
        }
        
        // Compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // More computations after the branch
        v21 = v1 + v2 + v3 + v4 + v5;
        v22 = v6 + v7 + v8 + v9 + v10;
        v23 = v11 + v12 + v13 + v14 + v15;
        v24 = v16 + v17 + v18 + v19 + v20;
        v25 = v21 + v22 + v23 + v24;
        
        // Use all variables to prevent dead code elimination
        v26 = v25 ^ v1 ^ v2 ^ v3;
        v27 = v4 ^ v5 ^ v6 ^ v7;
        v28 = v8 ^ v9 ^ v10 ^ v11;
        v29 = v12 ^ v13 ^ v14 ^ v15;
        v30 = v16 ^ v17 ^ v18 ^ v19;
        
        // Accumulate result
        result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
        
        // Modify inputs slightly for next iteration
        a = a ^ i;
        b = b + i;
        c = c - i;
        d = d ^ (i << 1);
        e = e + (i << 2);
        f = f - (i << 3);
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Initialize volatile variables with random values
    volatile int a = rand() % 1000;
    volatile int b = rand() % 1000;
    volatile int c = rand() % 1000;
    volatile int d = rand() % 1000;
    volatile int e = rand() % 1000;
    volatile int f = rand() % 1000;
    
    // Call the high-pressure function
    int result = high_pressure_compute(a, b, c, d, e, f);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}

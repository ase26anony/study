#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
        
        // Complex expression that will be reused - candidate for rematerialization
        int complex_expr = (a * b) + (c << 2) - d;
        
        // First block of independent computations
        v1 = a + b;
        v2 = b * c;
        v3 = c - d;
        v4 = d ^ a;
        v5 = a | b;
        v6 = b & c;
        v7 = c + d;
        v8 = d - a;
        v9 = a ^ b;
        v10 = b | c;
        
        // Use the complex expression multiple times
        v11 = complex_expr + v1;
        v12 = complex_expr - v2;
        v13 = complex_expr * v3;
        
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // Second block of computations
        v14 = v1 * v2;
        v15 = v3 + v4;
        v16 = v5 ^ v6;
        v17 = v7 | v8;
        v18 = v9 & v10;
        v19 = v11 - v12;
        v20 = v13 + v14;
        
        // Use complex expression again
        v21 = complex_expr + v15;
        v22 = complex_expr - v16;
        v23 = complex_expr * v17;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Third block with more computations
        v24 = v14 + v15;
        v25 = v16 * v17;
        v26 = v18 ^ v19;
        v27 = v20 | v21;
        v28 = v22 & v23;
        v29 = v24 - v25;
        v30 = v26 + v27;
        
        // Use complex expression one more time
        int temp1 = complex_expr + v28;
        int temp2 = complex_expr - v29;
        int temp3 = complex_expr * v30;
        
        // Conditional branch to split basic blocks
        volatile int condition = a > (b + i);
        if (condition) {
            // Additional computations in the taken branch
            v1 = v1 * 2;
            v2 = v2 + 3;
            v3 = v3 - 4;
            // Use complex expression again in this branch
            v4 = complex_expr + v1;
            v5 = complex_expr - v2;
            
            __asm__ volatile ("" : : : "memory");
            
            // More computations
            v6 = v4 * v5;
            v7 = v6 + complex_expr;
            result += v7;
        } else {
            // Different computations in the else branch
            v8 = v8 / 2;
            v9 = v9 * 3;
            v10 = v10 - 4;
            // Use complex expression here too
            v11 = complex_expr + v8;
            v12 = complex_expr - v9;
            
            __asm__ volatile ("" : : : "memory");
            
            // More computations
            v13 = v11 * v12;
            v14 = v13 - complex_expr;
            result += v14;
        }
        
        // Merge point - use values from both branches
        int merged = (condition ? v7 : v14) + temp1 + temp2 + temp3;
        result += merged & 0xFF;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // More independent computations to extend live ranges
        v15 = v1 + v2 + v3 + v4;
        v16 = v5 * v6 * v7 * v8;
        v17 = v9 ^ v10 ^ v11 ^ v12;
        v18 = v13 | v14 | v15 | v16;
        v19 = v17 & v18 & v19 & v20;
        v20 = v21 + v22 + v23 + v24;
        
        // Final use of complex expression
        v21 = complex_expr + v15;
        v22 = complex_expr - v16;
        v23 = complex_expr * v17;
        v24 = complex_expr ^ v18;
        
        result += v21 + v22 + v23 + v24;
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
    volatile int iterations = 10; // Small number to avoid long runtime
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    printf("Inputs: a=%d, b=%d, c=%d, d=%d\n", a, b, c, d);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure that should trigger early rematerialization
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iterations) {
    // Declare many local variables to create register pressure
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    int v31, v32, v33, v34, v35, v36, v37, v38, v39, v40;
    
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Use inline assembly as compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Complex expression that will be reused - potential rematerialization candidate
        int complex_expr = (a * b) + (c << 2) - d;
        
        // Force many independent computations to create register pressure
        v1 = a + b;
        v2 = b * c;
        v3 = c - d;
        v4 = d << 1;
        v5 = a ^ b;
        v6 = b | c;
        v7 = c & d;
        v8 = ~a;
        v9 = a * 3;
        v10 = b / 2;
        
        // Reuse the complex expression multiple times - this creates patterns
        // where rematerialization might be cheaper than spilling
        v11 = complex_expr + v1;
        v12 = complex_expr - v2;
        v13 = complex_expr * v3;
        v14 = complex_expr & v4;
        v15 = complex_expr | v5;
        
        // More computations
        v16 = v1 * v2 + v3;
        v17 = v4 - v5 * v6;
        v18 = v7 << (v8 & 3);
        v19 = v9 ^ v10;
        v20 = v11 | v12;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Create another complex expression for reuse
        int complex_expr2 = (v1 * v2) + (v3 << 3) - (v4 & 0xFF);
        
        // More computations reusing the second complex expression
        v21 = complex_expr2 + v13;
        v22 = complex_expr2 - v14;
        v23 = complex_expr2 * v15;
        v24 = complex_expr2 & v16;
        v25 = complex_expr2 | v17;
        
        // Even more computations
        v26 = v18 + v19 * v20;
        v27 = v21 - v22 / 2;
        v28 = v23 << (v24 & 7);
        v29 = v25 ^ v26;
        v30 = v27 | v28;
        
        // Conditional branch to split basic blocks
        volatile int condition = a & 1;
        if (condition) {
            // Branch 1: more computations
            v31 = v29 * v30 + complex_expr;
            v32 = v28 - v27 * complex_expr2;
            v33 = v26 << (v25 & 3);
            v34 = v24 ^ v23;
            v35 = v22 | v21;
            
            // Reuse complex expressions again
            v36 = complex_expr + complex_expr2;
            v37 = complex_expr - complex_expr2;
            v38 = complex_expr * complex_expr2;
            v39 = complex_expr & complex_expr2;
            v40 = complex_expr | complex_expr2;
            
            result += v31 + v32 + v33 + v34 + v35;
        } else {
            // Branch 2: different computations
            v31 = v20 * v19 + v18;
            v32 = v17 - v16 * v15;
            v33 = v14 << (v13 & 3);
            v34 = v12 ^ v11;
            v35 = v10 | v9;
            
            // Reuse complex expressions with different operations
            v36 = complex_expr2 + complex_expr;
            v37 = complex_expr2 - complex_expr;
            v38 = complex_expr2 * complex_expr;
            v39 = complex_expr2 & complex_expr;
            v40 = complex_expr2 | complex_expr;
            
            result += v36 + v37 + v38 + v39 + v40;
        }
        
        // Final compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Mix all results to prevent dead code elimination
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        result += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
        result += v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39 + v40;
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Use volatile variables to prevent constant propagation
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 10; // Small iteration count to avoid overflow
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

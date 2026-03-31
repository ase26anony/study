#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure - marked to prevent optimizations
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
        int complex_expr = (a * b) + (c << 2) - (d & 0xFF);
        
        // First block of independent computations
        v1 = a + b + i;
        v2 = b * c - i;
        v3 = c ^ d ^ i;
        v4 = d << (i & 3);
        v5 = (a * b) + (c << 2) - (d & 0xFF);  // Same as complex_expr
        v6 = v1 * v2 + v3;
        v7 = v4 | v5 | v6;
        v8 = (v1 + v2) * (v3 - v4);
        v9 = v5 ^ v6 ^ v7;
        v10 = (v8 << 2) + (v9 >> 1);
        
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // Second block with more computations
        v11 = complex_expr + v1;  // Use the complex expression
        v12 = complex_expr * v2;  // Use it again
        v13 = v3 + v4 + v5;
        v14 = v6 * v7 - v8;
        v15 = v9 ^ v10 ^ v11;
        v16 = v12 << (v13 & 3);
        v17 = (a * b) + (c << 2) - (d & 0xFF);  // Same expression again
        v18 = v14 + v15 + v16;
        v19 = v17 * v18;  // Use the recomputed value
        v20 = v19 & 0xFFFF;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Conditional block to split control flow
        if (a & 1) {
            // Branch with different computations
            v21 = v11 * v12 + v13;
            v22 = v14 - v15 * v16;
            v23 = (v17 >> 2) + (v18 << 1);
            v24 = v19 ^ v20 ^ v21;
            v25 = v22 * v23 - v24;
            v26 = complex_expr + v25;  // Reuse complex expression
            v27 = (a * b) + (c << 2) - (d & 0xFF);  // Recomputation
            v28 = v26 & v27;
            v29 = v28 | 0x1234;
            v30 = v29 * 2;
            
            result += v30;
        } else {
            // Alternative branch
            v21 = v11 + v12 - v13;
            v22 = v14 * v15 / (v16 + 1);
            v23 = (v17 << 1) | (v18 >> 1);
            v24 = v19 & v20 & v21;
            v25 = v22 + v23 + v24;
            v26 = complex_expr - v25;  // Reuse complex expression
            v27 = (a * b) + (c << 2) - (d & 0xFF);  // Recomputation
            v28 = v26 ^ v27;
            v29 = v28 & 0xABCD;
            v30 = v29 + 100;
            
            result -= v30;
        }
        
        // More computations after the conditional
        int temp1 = v21 * v22 + v23;
        int temp2 = v24 ^ v25 ^ v26;
        int temp3 = v27 & v28 & v29;
        int temp4 = v30 << (temp1 & 3);
        
        // Final complex expression reuse
        int final_expr = (a * b) + (c << 2) - (d & 0xFF);
        result += temp1 + temp2 - temp3 + temp4 + final_expr;
        
        // Another barrier
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Use volatile to prevent constant propagation
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 10;  // Small iteration count to avoid overflow
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

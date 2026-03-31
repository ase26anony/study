#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure - marked to prevent optimizations
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                          volatile int c, volatile int d,
                                          volatile int iterations) {
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Declare many local variables to create register pressure
        volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        volatile int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        // Complex expression that will be reused - candidate for rematerialization
        volatile int complex_expr = (a * b) + (c << 2) - (d / 3);
        
        // Force data dependencies and prevent optimization
        v1 = a + i;
        v2 = b - i;
        v3 = c * i;
        v4 = d ^ i;
        
        // Use the complex expression multiple times with different operations
        v5 = complex_expr + v1;
        v6 = complex_expr - v2;
        v7 = complex_expr * v3;
        v8 = complex_expr ^ v4;
        
        // More independent calculations to increase register pressure
        v9 = (v1 * v2) + (v3 << 1);
        v10 = (v2 * v3) - (v4 >> 1);
        v11 = (v3 * v4) ^ (v1 & 0xFF);
        v12 = (v4 * v1) | (v2 & 0x0F);
        
        // Compiler barrier - prevents reordering/coalescing
        __asm__ volatile ("" : : : "memory");
        
        // Another use of the complex expression
        v13 = complex_expr + v9;
        v14 = complex_expr - v10;
        
        // More calculations
        v15 = v5 * v6 + v7;
        v16 = v6 * v7 - v8;
        v17 = v7 * v8 ^ v5;
        v18 = v8 * v5 | v6;
        
        v19 = v9 + v10 * v11;
        v20 = v10 - v11 * v12;
        v21 = v11 ^ v12 * v9;
        v22 = v12 | v9 * v10;
        
        // Compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Conditional branch to split basic blocks
        volatile int condition = a & (1 << (i & 0x7));
        if (condition) {
            // Use complex expression again in the true branch
            v23 = complex_expr + v13;
            v24 = complex_expr - v14;
            v25 = v15 * 2 + v16;
            v26 = v17 ^ 0x55 + v18;
            
            // More calculations in the true path
            v27 = v19 + v20 * v21;
            v28 = v22 - v23 * v24;
            v29 = v25 ^ v26 * v27;
            v30 = v28 | v29 * v30;
            
            result += v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
        } else {
            // Different calculations in the false branch
            v23 = complex_expr * v13;
            v24 = complex_expr ^ v14;
            v25 = v15 + 7 * v16;
            v26 = v17 | 0xAA - v18;
            
            // More calculations in the false path
            v27 = v19 - v20 / (v21 + 1);
            v28 = v22 + v23 % (v24 + 1);
            v29 = v25 & v26 | v27;
            v30 = v28 ^ v29 & v30;
            
            result += v23 - v24 + v25 - v26 + v27 - v28 + v29 - v30;
        }
        
        // Final use of complex expression
        volatile int final_expr = complex_expr + (v1 ^ v2) - (v3 & v4);
        result += final_expr;
        
        // Compiler barrier at the end of loop iteration
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Initialize volatile variables with random values
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 10 + (rand() % 20); // 10-30 iterations
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    // Call the high-pressure function
    volatile int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

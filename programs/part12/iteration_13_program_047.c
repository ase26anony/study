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
        int complex_expr = (a * b) + (c << 2) - (d * 3);
        
        // First block of independent computations
        v1 = a + b + i;
        v2 = b * c - i;
        v3 = c ^ d ^ i;
        v4 = d << (a & 3);
        v5 = complex_expr + v1;  // First use of complex_expr
        v6 = (a * i) + (b * 2);
        v7 = (c | d) & 0xFF;
        v8 = v1 * v2 + v3;
        v9 = complex_expr - v4;  // Second use of complex_expr
        v10 = (v5 << 1) | (v6 >> 1);
        
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // Second block with more computations
        v11 = v1 + v2 + v3;
        v12 = v4 * v5 - v6;
        v13 = v7 ^ v8 ^ v9;
        v14 = v10 << (v1 & 3);
        v15 = complex_expr * v11;  // Third use of complex_expr
        v16 = (v2 * i) + (v3 * 2);
        v17 = (v4 | v5) & 0xFF;
        v18 = v12 * v13 + v14;
        v19 = complex_expr / (v15 + 1);  // Fourth use of complex_expr
        v20 = (v16 << 1) | (v17 >> 1);
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Third block with even more computations
        v21 = v11 + v12 + v13;
        v22 = v14 * v15 - v16;
        v23 = v17 ^ v18 ^ v19;
        v24 = v20 << (v11 & 3);
        v25 = complex_expr + v21;  // Fifth use of complex_expr
        v26 = (v12 * i) + (v13 * 2);
        v27 = (v14 | v15) & 0xFF;
        v28 = v22 * v23 + v24;
        v29 = complex_expr - v25;  // Sixth use of complex_expr
        v30 = (v26 << 1) | (v27 >> 1);
        
        // Control flow split based on volatile condition
        volatile int condition = a & 1;
        if (condition) {
            // Use all variables in true branch
            result += v1 + v3 + v5 + v7 + v9 + v11 + v13 + v15 + v17 + v19 +
                     v21 + v23 + v25 + v27 + v29;
        } else {
            // Use different variables in false branch
            result += v2 + v4 + v6 + v8 + v10 + v12 + v14 + v16 + v18 + v20 +
                     v22 + v24 + v26 + v28 + v30;
        }
        
        // Final compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Modify inputs slightly for next iteration
        a = (a + 1) & 0xFF;
        b = (b * 3) & 0xFF;
        c = (c ^ i) & 0xFF;
        d = (d - 1) & 0xFF;
    }
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    // Initialize volatile variables with random values
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 10;  // Small number to avoid long execution
    
    // Call the high-pressure computation function
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}

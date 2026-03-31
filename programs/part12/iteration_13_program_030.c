#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure that should trigger early rematerialization
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iterations) {
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Declare many local variables to create high register pressure
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        // Complex expression that will be reused - potential rematerialization candidate
        int common_expr = (a * b) + (c << 2) - (d * 3);
        
        // Force many independent computations to create register pressure
        v1 = a + b + i;
        v2 = b * c - i;
        v3 = c + d * 2;
        v4 = d - a / 2;
        v5 = common_expr + v1;  // First use of common_expr
        v6 = v1 * v2 + v3;
        v7 = v2 - v3 * v4;
        v8 = v3 + v4 / 2;
        v9 = common_expr - v2;  // Second use of common_expr
        v10 = v4 * v5 - v6;
        
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        v11 = v5 + v6 * 2;
        v12 = v6 - v7 / 3;
        v13 = v7 * v8 + 5;
        v14 = v8 - v9 * 2;
        v15 = common_expr * v3;  // Third use of common_expr
        v16 = v9 + v10 - 7;
        v17 = v10 * v11 / 4;
        v18 = v11 - v12 + 8;
        v19 = v12 * v13 - 9;
        v20 = common_expr + v4;  // Fourth use of common_expr
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        v21 = v13 + v14 * 3;
        v22 = v14 - v15 / 2;
        v23 = v15 * v16 + 11;
        v24 = v16 - v17 * 4;
        v25 = common_expr - v5;  // Fifth use of common_expr
        v26 = v17 + v18 / 3;
        v27 = v18 * v19 - 12;
        v28 = v19 - v20 + 13;
        v29 = v20 * v21 / 5;
        v30 = common_expr * v6;  // Sixth use of common_expr
        
        // Control flow split to complicate register allocation
        volatile int condition = a + i;
        if (condition > 100) {
            // More computations in the taken branch
            v1 = v1 + common_expr;  // Another use
            v3 = v3 * common_expr;  // Another use
            v5 = v5 - common_expr;  // Another use
            result += v1 + v3 + v5 + v30;
        } else {
            // Different computations in the else branch
            v2 = v2 + common_expr;  // Another use
            v4 = v4 * common_expr;  // Another use
            v6 = v6 - common_expr;  // Another use
            result += v2 + v4 + v6 + v25;
        }
        
        // Final barrier and result accumulation
        __asm__ volatile ("" : : : "memory");
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
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
    volatile int iterations = 10;  // Small number to avoid overflow
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

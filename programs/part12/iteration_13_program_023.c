#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure - marked to prevent optimizations
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                          volatile int c, volatile int d,
                                          volatile int iter_count) {
    volatile int result = 0;
    
    // Loop to create multiple basic blocks and extend live ranges
    for (volatile int i = 0; i < iter_count; i++) {
        // Memory barrier to prevent reordering/coalescing
        __asm__ volatile ("" : : : "memory");
        
        // Declare many local variables to create high register pressure
        // Each variable should be distinct to prevent optimization
        volatile int v1 = a + i;
        volatile int v2 = b - i;
        volatile int v3 = c * i;
        volatile int v4 = d ^ i;
        volatile int v5 = a * b;
        volatile int v6 = c + d;
        volatile int v7 = v1 ^ v2;
        volatile int v8 = v3 | v4;
        volatile int v9 = v5 << 2;
        volatile int v10 = v6 >> 1;
        volatile int v11 = v7 + v8;
        volatile int v12 = v9 - v10;
        volatile int v13 = v11 * v12;
        volatile int v14 = v13 & 0xFF;
        volatile int v15 = v14 + i;
        volatile int v16 = v15 * 3;
        volatile int v17 = v16 / 2;
        volatile int v18 = v17 | 0xAA;
        volatile int v19 = v18 ^ 0x55;
        volatile int v20 = v19 << 3;
        volatile int v21 = v20 >> 1;
        volatile int v22 = v21 + a;
        volatile int v23 = v22 - b;
        volatile int v24 = v23 * c;
        volatile int v25 = v24 / d;
        volatile int v26 = v25 ^ v1;
        volatile int v27 = v26 | v2;
        volatile int v28 = v27 & v3;
        volatile int v29 = v28 + v4;
        volatile int v30 = v29 - v5;
        
        // CRITICAL: Create a complex sub-expression that will be reused
        // This creates a rematerialization candidate
        volatile int complex_expr = (v1 * v2) + (v3 << 2) - (v4 & 0xF) + (v5 / 3);
        
        // Use the complex expression multiple times with different operations
        // This creates register copies that are candidates for rematerialization
        volatile int use1 = complex_expr + v6;
        volatile int use2 = complex_expr - v7;
        volatile int use3 = complex_expr * v8;
        volatile int use4 = complex_expr & v9;
        volatile int use5 = complex_expr | v10;
        volatile int use6 = complex_expr ^ v11;
        volatile int use7 = complex_expr + v12;
        volatile int use8 = complex_expr - v13;
        volatile int use9 = complex_expr * v14;
        volatile int use10 = complex_expr & v15;
        
        // Another memory barrier
        __asm__ volatile ("" : : : "memory");
        
        // Create conditional branch to split basic blocks
        // This complicates the control flow graph
        if (a & 1) {  // Use volatile 'a' to make condition unpredictable
            // Use all variables in this branch to keep them live
            volatile int branch_var = use1 + use2 + use3 + use4 + use5 +
                                     use6 + use7 + use8 + use9 + use10 +
                                     v16 + v17 + v18 + v19 + v20 +
                                     v21 + v22 + v23 + v24 + v25 +
                                     v26 + v27 + v28 + v29 + v30;
            result += branch_var;
        } else {
            // Different computation in else branch
            volatile int else_var = use1 - use2 + use3 - use4 + use5 -
                                   use6 + use7 - use8 + use9 - use10 +
                                   v16 - v17 + v18 - v19 + v20 -
                                   v21 + v22 - v23 + v24 - v25 +
                                   v26 - v27 + v28 - v29 + v30;
            result += else_var;
        }
        
        // Yet another memory barrier
        __asm__ volatile ("" : : : "memory");
        
        // More computations to extend live ranges
        volatile int final1 = use1 * use2;
        volatile int final2 = use3 * use4;
        volatile int final3 = use5 * use6;
        volatile int final4 = use7 * use8;
        volatile int final5 = use9 * use10;
        
        result += final1 + final2 + final3 + final4 + final5;
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
    volatile int iterations = 10;  // Small iteration count to avoid overflow
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    // Call the high-pressure function
    volatile int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

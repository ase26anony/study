#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure - marked to prevent optimizations
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                          volatile int c, volatile int d,
                                          volatile int e, volatile int f) {
    volatile int result = 0;
    volatile int iterations = 10; // Force loop execution
    
    // Complex expression that will be reused multiple times
    // This creates a rematerialization candidate
    volatile int base_expr = (a * b) + (c << 2) - (d / 3);
    
    for (volatile int i = 0; i < iterations; i++) {
        // Declare many local variables to create register pressure
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        // Use volatile inputs to prevent constant folding
        v1 = a + i;
        v2 = b - i;
        v3 = c * i;
        v4 = d ^ i;
        v5 = e | i;
        v6 = f & i;
        
        // Reuse the complex expression multiple times - creates copies
        // that are candidates for rematerialization
        v7 = base_expr + v1;
        v8 = base_expr - v2;
        v9 = base_expr * v3;
        v10 = base_expr ^ v4;
        
        // More computations to increase register pressure
        v11 = v1 * v2 + v3;
        v12 = v4 - v5 * v6;
        v13 = (v7 << 3) | (v8 >> 2);
        v14 = v9 ^ v10 & v11;
        v15 = v12 + v13 - v14;
        
        // Compiler barrier - prevents reordering/coalescing
        __asm__ volatile ("" : : : "memory");
        
        // More independent computations
        v16 = v15 * 7 + 13;
        v17 = v16 / 5 - 23;
        v18 = v17 << 1 | 0xFF;
        v19 = v18 ^ 0xABCD;
        v20 = v19 + 0x1234;
        
        // Conditional branch to split basic blocks
        volatile int condition = a > b;
        if (condition) {
            // Different computations in the true branch
            v21 = v20 * 3;
            v22 = v21 + 17;
            v23 = v22 - 29;
            v24 = v23 ^ 0xDEAD;
            v25 = v24 | 0xBEEF;
            
            // Another reuse of the base expression
            v26 = base_expr + v25;
            v27 = base_expr - v25;
            
            __asm__ volatile ("" : : : "memory");
        } else {
            // Different computations in the false branch
            v21 = v20 / 2;
            v22 = v21 * 19;
            v23 = v22 + 31;
            v24 = v23 & 0xF0F0;
            v25 = v24 ^ 0x0F0F;
            
            // Another reuse of the base expression
            v26 = base_expr * v25;
            v27 = base_expr / (v25 + 1);
            
            __asm__ volatile ("" : : : "memory");
        }
        
        // Merge point - values from both branches are used
        v28 = v26 + v27;
        v29 = v28 * i;
        v30 = v29 - base_expr; // Final reuse of base expression
        
        // Accumulate result
        result += v30;
        
        // Modify volatile inputs slightly for next iteration
        a += 1;
        b -= 1;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Initialize volatile inputs with random values
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int e = rand() % 100 + 1;
    volatile int f = rand() % 100 + 1;
    
    // Call the high-pressure function
    volatile int result = high_pressure_compute(a, b, c, d, e, f);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}

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
        volatile int v1 = a + i;
        volatile int v2 = b - i;
        volatile int v3 = c * i;
        volatile int v4 = d ^ i;
        volatile int v5 = a * b;
        volatile int v6 = c + d;
        volatile int v7 = v1 * v2;
        volatile int v8 = v3 << 2;
        volatile int v9 = v4 >> 1;
        volatile int v10 = v5 & v6;
        volatile int v11 = v7 | v8;
        volatile int v12 = v9 ^ v10;
        volatile int v13 = v11 + v12;
        volatile int v14 = v13 * 3;
        volatile int v15 = v14 / 2;
        volatile int v16 = v15 % 7;
        volatile int v17 = v16 << 3;
        volatile int v18 = v17 >> 1;
        volatile int v19 = v18 | 0xFF;
        volatile int v20 = v19 & 0x0F;
        
        // CRITICAL: Create a complex sub-expression that will be reused
        // This creates a rematerialization candidate
        volatile int common_expr = (v1 * v2) + (v3 << 2) - (v4 / 3);
        
        // Use the common expression multiple times with different operations
        // This creates register copies that could be rematerialized
        volatile int copy1 = common_expr;
        volatile int copy2 = common_expr;
        volatile int copy3 = common_expr;
        volatile int copy4 = common_expr;
        volatile int copy5 = common_expr;
        
        // Perform operations on the copies to keep them live
        volatile int t1 = copy1 + v5;
        volatile int t2 = copy2 * v6;
        volatile int t3 = copy3 - v7;
        volatile int t4 = copy4 ^ v8;
        volatile int t5 = copy5 | v9;
        
        // Compiler barrier - prevents reordering/optimization across this point
        __asm__ volatile ("" : : : "memory");
        
        // More variables to increase pressure
        volatile int u1 = t1 * t2;
        volatile int u2 = t3 + t4;
        volatile int u3 = t5 ^ t1;
        volatile int u4 = u1 & u2;
        volatile int u5 = u3 | u4;
        volatile int u6 = u5 << 1;
        volatile int u7 = u6 >> 2;
        volatile int u8 = u7 * 3;
        volatile int u9 = u8 / 5;
        volatile int u10 = u9 % 11;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Control flow split - creates basic block boundaries
        volatile int condition = (i % 3) == 0;
        if (condition) {
            // Branch creates separate live ranges
            volatile int branch1 = u1 + u2;
            volatile int branch2 = u3 * u4;
            volatile int branch3 = branch1 ^ branch2;
            volatile int branch4 = branch3 << 2;
            volatile int branch5 = branch4 >> 1;
            
            // Reuse common_expr again in the branch
            volatile int branch_copy = common_expr;
            result += branch_copy + branch5;
            
            __asm__ volatile ("" : : : "memory");
        } else {
            // Alternative path with different computations
            volatile int else1 = u5 + u6;
            volatile int else2 = u7 * u8;
            volatile int else3 = else1 ^ else2;
            volatile int else4 = else3 << 1;
            volatile int else5 = else4 >> 3;
            
            // Another reuse of common_expr
            volatile int else_copy = common_expr;
            result += else_copy - else5;
            
            __asm__ volatile ("" : : : "memory");
        }
        
        // Final computations mixing many variables
        volatile int final1 = v10 + v11 + v12;
        volatile int final2 = v13 * v14 * v15;
        volatile int final3 = v16 ^ v17 ^ v18;
        volatile int final4 = v19 | v20 | t1;
        volatile int final5 = t2 + t3 + t4;
        
        // One more reuse of the common expression
        volatile int final_copy = common_expr;
        result += final1 + final2 + final3 + final4 + final5 + final_copy;
        
        // Final compiler barrier
        __asm__ volatile ("" : : : "memory");
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
    volatile int iterations = 10; // Small enough to run, large enough for pressure
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    volatile int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

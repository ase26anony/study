#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Prevent inlining and interprocedural optimization
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int e, volatile int f,
                                 volatile int iter_count) {
    volatile int result = 0;
    
    // Loop with volatile iteration count to prevent unrolling
    for (volatile int i = 0; i < iter_count; i++) {
        // Declare many local variables to create register pressure
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        // Complex expression that will be reused multiple times
        // This creates rematerialization candidates
        int common_expr = (a * b) + (c << 2) - (d & e) | (f ^ 0x7F);
        
        // First block of independent computations
        v1 = a + b + common_expr;
        v2 = b * c - common_expr;
        v3 = c ^ d ^ common_expr;
        v4 = d | e | common_expr;
        v5 = e & f & common_expr;
        v6 = a - b + common_expr;
        v7 = b - c - common_expr;
        v8 = c * d * common_expr;
        v9 = d ^ e ^ common_expr;
        v10 = e | f | common_expr;
        
        // Compiler barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // Reuse the common expression again
        int common_expr2 = common_expr;  // This creates a register copy
        
        // More computations using the copied expression
        v11 = f + a + common_expr2;
        v12 = a * f - common_expr2;
        v13 = b ^ c ^ common_expr2;
        v14 = c | d | common_expr2;
        v15 = d & e & common_expr2;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Create control flow split based on volatile condition
        volatile int condition = a & 1;
        if (condition) {
            // Branch 1: More computations with common expression
            v16 = v1 + v2 + common_expr;
            v17 = v3 * v4 - common_expr;
            v18 = v5 ^ v6 ^ common_expr;
            v19 = v7 | v8 | common_expr;
            v20 = v9 & v10 & common_expr;
            
            // Recompute common_expr in a different way
            int common_expr3 = (b * a) + (c << 2) - (e & d) | (f ^ 0x7F);
            v21 = v11 + v12 + common_expr3;
            v22 = v13 * v14 - common_expr3;
            
            // Another copy of the expression
            int common_expr4 = common_expr3;
            v23 = v15 ^ v16 ^ common_expr4;
            v24 = v17 | v18 | common_expr4;
            
        } else {
            // Branch 2: Alternative computations
            v16 = v2 - v1 + common_expr;
            v17 = v4 / (v3 + 1) - common_expr;
            v18 = v6 ^ v5 ^ common_expr;
            v19 = v8 | v7 | common_expr;
            v20 = v10 & v9 & common_expr;
            
            // Another instance of the common expression
            int common_expr5 = common_expr;
            v21 = v12 + v11 + common_expr5;
            v22 = v14 * v13 - common_expr5;
            
            // Yet another copy
            int common_expr6 = common_expr5;
            v23 = v16 ^ v15 ^ common_expr6;
            v24 = v18 | v17 | common_expr6;
        }
        
        // Merge point - use all variables to keep them live
        v25 = v1 + v2 + v3 + v4 + v5;
        v26 = v6 + v7 + v8 + v9 + v10;
        v27 = v11 + v12 + v13 + v14 + v15;
        v28 = v16 + v17 + v18 + v19 + v20;
        v29 = v21 + v22 + v23 + v24;
        
        // Final computation using many variables
        v30 = v25 * v26 - v27 / (v28 + 1) + v29;
        
        // Accumulate result
        result += v30;
        
        // Modify inputs slightly for next iteration
        a ^= 0x01;
        b += 0x02;
        c -= 0x03;
        d |= 0x04;
        e &= 0xFC;
        f ^= 0x05;
        
        // Compiler barrier at end of loop
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
    volatile int e = rand() % 100 + 1;
    volatile int f = rand() % 100 + 1;
    volatile int iter_count = 10;  // Small iteration count
    
    // Call the high-pressure function
    int result = high_pressure_compute(a, b, c, d, e, f, iter_count);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}

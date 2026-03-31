#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure that should trigger early rematerialization
__attribute__((noinline, noipa))
volatile int high_pressure_compute(volatile int a, volatile int b, volatile int c, 
                                   volatile int d, volatile int e, volatile int f) {
    volatile int result = 0;
    volatile int iterations = (a % 10) + 15; // Force multiple loop iterations
    
    for (volatile int i = 0; i < iterations; i++) {
        // Create many local variables to increase register pressure
        int v1 = a + i;
        int v2 = b - i;
        int v3 = c * i;
        int v4 = d ^ i;
        int v5 = e << (i & 3);
        int v6 = f >> (i & 3);
        int v7 = v1 * v2;
        int v8 = v3 + v4;
        int v9 = v5 | v6;
        int v10 = v7 & v8;
        int v11 = v9 ^ v10;
        int v12 = v1 + v2 + v3;
        int v13 = v4 * v5 * v6;
        int v14 = v7 - v8 + v9;
        int v15 = v10 | v11 | v12;
        int v16 = v13 & v14 & v15;
        int v17 = v1 << 2;
        int v18 = v2 >> 1;
        int v19 = v3 * 3;
        int v20 = v4 + 7;
        int v21 = v5 - 5;
        int v22 = v6 ^ 0xFF;
        int v23 = v7 * 2;
        int v24 = v8 / 2;
        int v25 = v9 + 100;
        int v26 = v10 - 50;
        int v27 = v11 * 4;
        int v28 = v12 | 0x0F;
        int v29 = v13 & 0xF0;
        int v30 = v14 ^ 0x55;
        
        // Complex sub-expression that will be reused - candidate for rematerialization
        // This creates a pattern where recomputing might be cheaper than spilling
        int complex_expr = (v1 * v2) + (v3 << 2) - (v4 & 0xF) + (v5 | 0x1);
        
        // Use the complex expression multiple times with different operations
        // This creates register copies that might be replaced with recomputation
        int copy1 = complex_expr;
        int copy2 = complex_expr;
        int copy3 = complex_expr;
        int copy4 = complex_expr;
        int copy5 = complex_expr;
        
        // Perform operations on the copies to keep them live
        v1 = copy1 + v17;
        v2 = copy2 - v18;
        v3 = copy3 * v19;
        v4 = copy4 ^ v20;
        v5 = copy5 | v21;
        
        // Compiler barrier to prevent reordering/optimization across this point
        __asm__ volatile ("" : : : "memory");
        
        // More computations to maintain high register pressure
        int v31 = v22 + v23;
        int v32 = v24 * v25;
        int v33 = v26 ^ v27;
        int v34 = v28 & v29;
        int v35 = v30 | v31;
        int v36 = v32 - v33;
        int v37 = v34 + v35;
        int v38 = v36 * v37;
        int v39 = v1 + v2 + v3 + v4 + v5;
        int v40 = v6 * v7 * v8 * v9 * v10;
        
        // Another complex expression reused
        int complex_expr2 = (v11 << 3) | (v12 & 0x7F) ^ (v13 * 2) + (v14 % 17);
        int copy6 = complex_expr2;
        int copy7 = complex_expr2;
        int copy8 = complex_expr2;
        
        // Conditional branch to split basic blocks
        volatile int condition = (i & 1);
        if (condition) {
            // Different computation path
            v31 = copy6 + v15;
            v32 = copy7 - v16;
            v33 = copy8 * v17;
            result += v31 + v32 + v33;
        } else {
            // Alternative path
            v34 = copy6 ^ v18;
            v35 = copy7 | v19;
            v36 = copy8 & v20;
            result += v34 + v35 + v36;
        }
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Final set of computations
        int v41 = v21 + v22 + v23;
        int v42 = v24 * v25 * v26;
        int v43 = v27 ^ v28 ^ v29;
        int v44 = v30 & v31 & v32;
        int v45 = v33 | v34 | v35;
        int v46 = v36 + v37 + v38;
        int v47 = v39 * v40 * v41;
        int v48 = v42 - v43 - v44;
        int v49 = v45 ^ v46 ^ v47;
        int v50 = v48 & v49;
        
        // Accumulate results
        result += v50 + complex_expr + complex_expr2;
        
        // Force another compiler barrier
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Initialize volatile variables with random values
    volatile int v1 = rand() % 100 + 1;
    volatile int v2 = rand() % 100 + 1;
    volatile int v3 = rand() % 100 + 1;
    volatile int v4 = rand() % 100 + 1;
    volatile int v5 = rand() % 100 + 1;
    volatile int v6 = rand() % 100 + 1;
    
    // Call the high-pressure function
    volatile int result = high_pressure_compute(v1, v2, v3, v4, v5, v6);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}

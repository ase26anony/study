#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure - marked to prevent optimizations
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                          volatile int c, volatile int d,
                                          volatile int e, volatile int f) {
    // Local variables - many to create register pressure
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    int v31, v32, v33, v34, v35, v36, v37, v38, v39, v40;
    
    // Complex expression that will be reused - rematerialization candidate
    int common_expr1, common_expr2, common_expr3, common_expr4;
    
    // Initialize with arithmetic to create dependencies
    v1 = a + b;
    v2 = c - d;
    v3 = e * f;
    v4 = a ^ b;
    v5 = c | d;
    v6 = e & f;
    
    // Memory barrier to prevent reordering
    __asm__ volatile ("" : : : "memory");
    
    // Create the common sub-expression: (a*b) + (c<<2) - complex enough
    common_expr1 = (a * b) + (c << 2);
    
    // Copy the common expression to different variables
    // This creates register-to-register moves that are rematerialization candidates
    common_expr2 = common_expr1;
    common_expr3 = common_expr2;
    common_expr4 = common_expr3;
    
    // More computations using the common expression
    v7 = common_expr1 + v1;
    v8 = common_expr2 - v2;
    v9 = common_expr3 * v3;
    v10 = common_expr4 ^ v4;
    
    __asm__ volatile ("" : : : "memory");
    
    // Additional independent computations
    v11 = v1 * v2 + v3;
    v12 = v4 | v5 & v6;
    v13 = (v7 << 3) ^ (v8 >> 1);
    v14 = v9 * 17 - v10;
    v15 = (v11 + v12) * (v13 - v14);
    
    // Another memory barrier
    __asm__ volatile ("" : : : "memory");
    
    // More variables and computations
    v16 = a * 31 + b * 7;
    v17 = c * 13 - d * 11;
    v18 = e * 5 + f * 3;
    v19 = v16 ^ v17 | v18;
    v20 = (v16 + v17) * (v18 - v19);
    
    // Reuse the common expression again
    v21 = common_expr1 + v16;
    v22 = common_expr2 - v17;
    v23 = common_expr3 * v18;
    v24 = common_expr4 ^ v19;
    
    __asm__ volatile ("" : : : "memory");
    
    // Even more variables
    v25 = v20 * 2 + v21;
    v26 = v22 * 3 - v23;
    v27 = v24 * 5 + v25;
    v28 = v26 * 7 - v27;
    v29 = v28 * 11 + v29;  // Self-reference creates additional complexity
    v30 = v29 * 13 - v30;
    
    // Control flow to split basic blocks
    volatile int condition = a > b;
    if (condition) {
        // Branch 1 - more computations
        v31 = v1 * v3 + v5;
        v32 = v2 * v4 - v6;
        v33 = v31 ^ v32 | v7;
        v34 = v33 * 19 + v8;
        
        // Reuse common expression in this branch
        v35 = common_expr1 + v31;
        v36 = common_expr2 - v32;
        
        __asm__ volatile ("" : : : "memory");
    } else {
        // Branch 2 - different computations
        v31 = v3 * v5 + v1;
        v32 = v4 * v6 - v2;
        v33 = v31 & v32 | v9;
        v34 = v33 * 23 - v10;
        
        // Reuse common expression here too
        v35 = common_expr3 + v31;
        v36 = common_expr4 - v32;
        
        __asm__ volatile ("" : : : "memory");
    }
    
    // Merge point - computations using variables from both branches
    v37 = v31 * v32 + v33;
    v38 = v34 ^ v35 | v36;
    v39 = v37 * 29 - v38;
    v40 = v39 * 31 + v40;  // Another self-reference
    
    // Final computation using many variables
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                 v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39 + v40 +
                 common_expr1 + common_expr2 + common_expr3 + common_expr4;
    
    return result;
}

// Loop-based version to increase pressure further
__attribute__((noinline, noipa))
static volatile int loop_high_pressure(volatile int iter) {
    volatile int sum = 0;
    
    for (volatile int i = 0; i < iter; i++) {
        // Create many temporary variables inside the loop
        int t1 = i * 2 + 1;
        int t2 = i * 3 - 2;
        int t3 = t1 * t2;
        int t4 = t1 ^ t2;
        int t5 = t3 | t4;
        int t6 = t5 << (i & 3);
        int t7 = t6 >> 1;
        int t8 = t7 * 7 + t6;
        int t9 = t8 - t5 * 3;
        int t10 = t9 ^ t4;
        
        // Common expression inside loop
        int loop_common = (t1 * t3) + (t2 << 2);
        int loop_common_copy1 = loop_common;
        int loop_common_copy2 = loop_common_copy1;
        int loop_common_copy3 = loop_common_copy2;
        
        // Use the common expression
        int t11 = loop_common + t1;
        int t12 = loop_common_copy1 - t2;
        int t13 = loop_common_copy2 * t3;
        int t14 = loop_common_copy3 ^ t4;
        
        // Memory barrier in loop
        __asm__ volatile ("" : : : "memory");
        
        // More variables
        int t15 = t11 * t12;
        int t16 = t13 | t14;
        int t17 = t15 + t16;
        int t18 = t17 * 11;
        int t19 = t18 - t15;
        int t20 = t19 ^ t16;
        
        // Conditional in loop
        if (i & 1) {
            int t21 = t17 * 2;
            int t22 = t18 / 3;
            int t23 = t21 + t22;
            sum += t23;
            
            // Reuse common expression
            int t24 = loop_common + t21;
            sum += t24;
        } else {
            int t21 = t19 * 4;
            int t22 = t20 / 5;
            int t23 = t21 - t22;
            sum -= t23;
            
            // Reuse common expression
            int t24 = loop_common_copy1 - t21;
            sum += t24;
        }
        
        // Final loop computation
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
               t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
    }
    
    return sum;
}

int main() {
    srand(time(NULL));
    
    // Volatile inputs to prevent constant propagation
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int e = rand() % 100 + 1;
    volatile int f = rand() % 100 + 1;
    volatile int iterations = 10 + (rand() % 20);
    
    printf("Inputs: a=%d, b=%d, c=%d, d=%d, e=%d, f=%d, iter=%d\n",
           a, b, c, d, e, f, iterations);
    
    // Call high pressure functions
    volatile int result1 = high_pressure_compute(a, b, c, d, e, f);
    volatile int result2 = loop_high_pressure(iterations);
    
    volatile int final_result = result1 + result2;
    
    printf("Result1: %d\n", result1);
    printf("Result2: %d\n", result2);
    printf("Final result: %d\n", final_result);
    
    return 0;
}

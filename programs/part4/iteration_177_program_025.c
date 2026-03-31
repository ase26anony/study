#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))

// Helper functions that won't be inlined
NOINLINE int helper1(int a, int b, int c) {
    volatile int result = (a ^ b) | c;
    asm volatile("" ::: "memory");
    return result * 31 + 17;
}

NOINLINE float helper2(float x, float y, int scale) {
    volatile float temp = x * y + (float)scale;
    asm volatile("" ::: "memory");
    return temp / 3.14159f;
}

NOINLINE double helper3(double a, double b, int* ptr) {
    volatile double sum = a + b;
    *ptr += (int)sum;
    asm volatile("" ::: "memory");
    return sum * 0.5;
}

NOINLINE void memory_op(int* dest, const int* src, int count) {
    volatile int acc = 0;
    for (int i = 0; i < count; i++) {
        dest[i] = src[i] + acc;
        acc ^= src[i];
        asm volatile("" ::: "memory");
    }
}

int main() {
    // High register pressure: many local variables of different types
    volatile int outer_limit = 1000;
    volatile int inner_limit = 50;
    volatile int switch_var = 0;
    
    // Integer variables
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    // Floating point variables
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    // Double precision variables
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    // Pointer variables
    int* ptr1 = &v1;
    int* ptr2 = &v2;
    int* ptr3 = &v3;
    
    // Arrays for memory operations
    int arr1[100], arr2[100];
    for (int i = 0; i < 100; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
    }
    
    // Final checksum
    volatile uint64_t checksum = 0;
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        // Mixed operation dependency chain
        v1 = v2 + v3 * v4;
        f1 = (float)v1 * f2;
        asm volatile("" ::: "memory");
        
        d1 = (double)f1 + d2 * 1.5;
        v5 = (int)d1 ^ v6;
        asm volatile("" ::: "memory");
        
        // Nested loops with variable bounds
        volatile int mid_limit = outer % 20 + 10;
        for (volatile int mid = 0; mid < mid_limit; mid++) {
            // Inner loop with volatile-dependent bound
            volatile int inner_bound = (mid * 3) % 15 + 5;
            for (volatile int inner = 0; inner < inner_bound; inner++) {
                // Complex dependency chain across different types
                v7 = v8 * v9 - v10;
                f3 = f4 / (float)v7 + f5;
                asm volatile("" ::: "memory");
                
                d3 = (double)v7 * d4 - (double)f3;
                v11 = (int)d3 | v12;
                asm volatile("" ::: "memory");
                
                // Memory operations creating load/store dependencies
                arr1[inner % 100] = v11 + arr2[inner % 100];
                v13 = arr1[(inner + 1) % 100] * 2;
                asm volatile("" ::: "memory");
                
                // Function calls with scheduling side effects
                v14 = helper1(v13, v14, inner);
                f6 = helper2(f6, f7, v14);
                asm volatile("" ::: "memory");
                
                d5 = helper3(d5, d3, &v15);
                v16 = (int)d5 & v17;
                asm volatile("" ::: "memory");
                
                // Conditional execution paths
                switch_var = (switch_var + 1) % 4;
                switch (switch_var) {
                    case 0:
                        // FP math branch
                        f8 = f9 * f10 + (float)v18;
                        v19 = (int)f8 * v20;
                        f9 = helper2(f8, f9, v19);
                        break;
                    case 1:
                        // Integer bit manipulation branch
                        v18 = (v19 ^ v20) | (v1 << 2);
                        v19 = helper1(v18, v19, v20);
                        v20 = v18 & v19 | v20;
                        break;
                    case 2:
                        // Memory intensive branch
                        memory_op(arr1, arr2, 20);
                        v1 = arr1[0] + arr2[0];
                        break;
                    case 3:
                        // Mixed operations branch
                        d2 = (double)v2 * 1.618;
                        f10 = (float)d2 / 2.0f;
                        v3 = (int)f10 * v4;
                        d4 = helper3(d2, d4, &v5);
                        break;
                }
                asm volatile("" ::: "memory");
                
                // More register pressure operations
                v21 = v22 * v23 - v24;
                f4 = f5 * (float)v21 + f6;
                v25 = v26 | v27 & v28;
                d1 = (double)v25 * 0.333;
                asm volatile("" ::: "memory");
                
                // Update checksum with various values
                checksum ^= (uint64_t)v1;
                checksum ^= (uint64_t)(*(uint32_t*)&f1);
                checksum ^= (uint64_t)(*(uint64_t*)&d1);
                checksum ^= (uint64_t)v21;
            }
            
            // Additional operations between inner loops
            v29 = helper1(v25, v26, mid);
            f7 = helper2(f7, f8, v29);
            asm volatile("" ::: "memory");
            
            // Memory barrier
            asm volatile("" ::: "memory");
        }
        
        // Update volatile variables to prevent optimization
        outer_limit = outer_limit; // Prevent dead store elimination
        switch_var = (switch_var + outer) % 4;
        
        // More complex dependency chains
        v30 = v31 * v32 + v33;
        f5 = (float)v30 * f6 - f7;
        d2 = (double)f5 + d3 * d4;
        v34 = (int)d2 ^ v35;
        asm volatile("" ::: "memory");
        
        // Call helper with many live variables
        v36 = helper1(v34, v35, v36);
        f8 = helper2(f7, f8, v36);
        d5 = helper3(d4, d5, &v37);
        asm volatile("" ::: "memory");
    }
    
    // Final memory operation
    memory_op(arr1, arr2, 50);
    
    // Final checksum calculation including all variables
    checksum ^= (uint64_t)v1 ^ (uint64_t)v10 ^ (uint64_t)v20;
    checksum ^= (uint64_t)v30 ^ (uint64_t)v36 ^ (uint64_t)v37;
    checksum ^= (uint64_t)(*(uint32_t*)&f1) ^ (uint64_t)(*(uint32_t*)&f10);
    checksum ^= (uint64_t)(*(uint64_t*)&d1) ^ (uint64_t)(*(uint64_t*)&d5);
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}

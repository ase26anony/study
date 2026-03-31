#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Non-inline helper functions to force scheduler state saves/restores
__attribute__((noinline)) static float helper_float_op(float a, float b, float c) {
    volatile float barrier = a * b;  // Prevent optimization
    asm volatile("" ::: "memory");
    return barrier + c * 0.5f;
}

__attribute__((noinline)) static int helper_int_op(int a, int b, int c) {
    volatile int barrier = a ^ b;  // Prevent optimization
    asm volatile("" ::: "memory");
    return (barrier * c) >> 3;
}

__attribute__((noinline)) static double helper_double_op(double a, double b, int scale) {
    volatile double barrier = a / (b + 1.0);
    asm volatile("" ::: "memory");
    return barrier * scale;
}

__attribute__((noinline)) static void helper_memory_op(int* arr, float* farr, int idx) {
    volatile int temp = arr[idx];
    asm volatile("" ::: "memory");
    farr[idx] = temp * 0.25f;
    asm volatile("" ::: "memory");
}

// Main complex function with high register pressure
static uint64_t complex_scheduling_function(void) {
    // Declare many variables to create high register pressure
    volatile int outer_limit = 1000;  // Prevent constant propagation
    
    // Integer variables (15+)
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    // Floating point variables (10+)
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    // Double precision variables
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    // Array for memory operations
    int int_array[64];
    float float_array[64];
    
    // Initialize arrays
    for (int i = 0; i < 64; i++) {
        int_array[i] = i * 3;
        float_array[i] = i * 0.5f;
    }
    
    uint64_t checksum = 0;
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        // Mixed operation dependency chain
        v1 = v2 + v3 * v4;
        f1 = f2 * f3 + f4;
        asm volatile("" ::: "memory");  // Barrier
        
        // Nested loop with variable bounds
        volatile int inner_limit = (outer % 16) + 8;  // Variable trip count
        for (int inner = 0; inner < inner_limit; inner++) {
            // Complex dependency chain across types
            d1 = helper_double_op(d2, d3, v1);
            v5 = helper_int_op(v6, v7, inner);
            f5 = helper_float_op(f6, f7, f8);
            
            // Memory operations with dependencies
            int idx = (v5 + inner) & 63;
            helper_memory_op(int_array, float_array, idx);
            
            // More mixed operations
            v8 = v9 ^ v10;
            f9 = f10 * 1.5f;
            d2 = d1 * 0.99;
            
            asm volatile("" ::: "memory");  // Barrier
            
            // Conditional execution paths
            switch (inner & 7) {
                case 0:
                    // FP math path
                    f1 = f2 * f3 - f4;
                    d3 = d4 / d5;
                    v11 = v12 * v13;
                    break;
                case 1:
                    // Integer bit manipulation path
                    v14 = (v15 << 3) | (v16 >> 2);
                    v17 = v18 ^ v19 ^ v20;
                    f5 = (float)v14 * 0.01f;
                    break;
                case 2:
                    // Memory intensive path
                    for (int j = 0; j < 4; j++) {
                        int mem_idx = (inner + j) & 63;
                        int_array[mem_idx] += v1;
                        float_array[mem_idx] *= 1.01f;
                    }
                    break;
                case 3:
                    // Mixed operations
                    v2 = helper_int_op(v3, v4, v5);
                    f6 = helper_float_op(f7, f8, f9);
                    d4 = helper_double_op(d5, d1, v2);
                    break;
                case 4:
                    // Another dependency chain
                    v6 = v7 * v8 - v9;
                    f10 = f1 + f2 * f3;
                    v10 = (v11 << 2) | (v12 >> 1);
                    break;
                case 5:
                    // More FP operations
                    f3 = f4 * 2.0f - f5;
                    f4 = f6 / 3.0f + f7;
                    d5 = d1 * d2 - d3;
                    break;
                case 6:
                    // Integer operations
                    v13 = v14 * v15 >> 1;
                    v16 = v17 ^ v18 + v19;
                    v20 = v1 * v2 % 1023;
                    break;
                case 7:
                    // All types
                    v1 = v2 + v3;
                    f8 = f9 * 0.75f;
                    d1 = d2 + 1.5;
                    helper_memory_op(int_array, float_array, inner & 63);
                    break;
            }
            
            asm volatile("" ::: "memory");  // Barrier
            
            // Update checksum with various values
            checksum ^= (uint64_t)v1;
            checksum ^= (uint64_t)(*(uint32_t*)&f1);
            checksum ^= (uint64_t)(*(uint64_t*)&d1);
            checksum ^= (uint64_t)int_array[inner & 63];
        }
        
        // Deeply nested third loop level
        for (int deep = 0; deep < 4; deep++) {
            volatile int deep_limit = (outer + deep) & 7;  // Unpredictable
            
            for (int very_deep = 0; very_deep < deep_limit; very_deep++) {
                // Interleaved operations creating long dependency chains
                v1 = v2 * v3 + v4;
                f1 = helper_float_op(f2, f3, f4);
                v5 = v6 ^ v7 | v8;
                d1 = d2 * d3 - d4;
                f5 = f6 / f7 + f8;
                v9 = helper_int_op(v10, v11, v12);
                
                asm volatile("" ::: "memory");  // Barrier
                
                // Update arrays
                int idx = (v1 + very_deep) & 63;
                int_array[idx] += v5;
                float_array[idx] = f5 * 0.5f;
                
                // More operations
                v13 = v14 * v15 >> 2;
                f9 = f10 * 1.1f;
                d5 = helper_double_op(d1, d2, v13);
                
                checksum ^= (uint64_t)v13;
            }
        }
        
        // Final mixed operation sequence in outer loop
        v2 = v3 * v4 - v5;
        f2 = f3 + f4 * f5;
        d2 = d3 / d4 + d5;
        v6 = v7 ^ v8 ^ v9;
        f6 = helper_float_op(f7, f8, f9);
        v10 = helper_int_op(v11, v12, v13);
        d3 = helper_double_op(d4, d5, v10);
        
        asm volatile("" ::: "memory");  // Barrier
    }
    
    // Final accumulation from arrays
    for (int i = 0; i < 64; i++) {
        checksum ^= (uint64_t)int_array[i];
        checksum ^= (uint64_t)(*(uint32_t*)&float_array[i]);
    }
    
    return checksum;
}

int main(void) {
    printf("Starting complex scheduling stress test...\n");
    
    uint64_t result = complex_scheduling_function();
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}

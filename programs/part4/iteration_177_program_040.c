#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Prevent inlining to force scheduler state saves/restores
__attribute__((noinline)) 
int helper_func1(int a, int b, float c, double d) {
    volatile int barrier = 0;
    asm volatile("" : "+r"(barrier) : : "memory");
    
    int result = (a * b) + (int)(c * 10.0f) + (int)(d * 5.0);
    asm volatile("" : : : "memory");
    
    // Mix operations
    float temp_f = (float)result * 0.5f;
    double temp_d = (double)temp_f * 1.5;
    result = (int)(temp_d * 2.0) ^ (a & b);
    
    return result;
}

__attribute__((noinline))
float helper_func2(float a, float b, int c, int d) {
    volatile float barrier = 0.0f;
    asm volatile("" : "+f"(barrier) : : "memory");
    
    float result = a * b + (float)(c ^ d);
    
    // Create FP dependency chain
    for (int i = 0; i < 3; i++) {
        result = result * 1.1f - 0.5f;
        asm volatile("" : : : "memory");
    }
    
    return result;
}

__attribute__((noinline))
void memory_ops(int* arr, float* farr, double* darr, int idx) {
    volatile int v_idx = idx;
    asm volatile("" : "+r"(v_idx) : : "memory");
    
    // Complex memory access pattern
    arr[v_idx % 64] = arr[(v_idx + 1) % 64] ^ arr[(v_idx + 31) % 64];
    farr[v_idx % 32] = farr[(v_idx + 7) % 32] * 0.9f;
    darr[v_idx % 16] = darr[(v_idx + 3) % 16] * 1.1;
    
    asm volatile("" : : : "memory");
}

// Complex main function with high register pressure
int main() {
    // Many local variables to create register pressure (30+)
    volatile int outer_limit = 1000;  // Prevent constant propagation
    
    // Integer variables
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    // Floating point variables
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    // Double variables
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    // Arrays for memory operations
    int int_arr[64];
    float float_arr[32];
    double double_arr[16];
    
    // Initialize arrays
    for (int i = 0; i < 64; i++) int_arr[i] = i;
    for (int i = 0; i < 32; i++) float_arr[i] = i * 0.5f;
    for (int i = 0; i < 16; i++) double_arr[i] = i * 0.25;
    
    volatile int checksum = 0;
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        // Nested loop with variable bounds
        volatile int inner_limit = (outer % 10) + 5;
        
        for (volatile int inner = 0; inner < inner_limit; inner++) {
            // Mixed operation dependency chain
            v1 = v2 + v3;
            f1 = (float)v1 * 0.5f;
            asm volatile("" : : : "memory");
            
            d1 = (double)f1 * 1.5;
            v4 = (int)d1 ^ v5;
            
            // Call helper with cross-type dependencies
            v6 = helper_func1(v4, v7, f2, d2);
            asm volatile("" : : : "memory");
            
            f3 = helper_func2(f1, f4, v8, v9);
            
            // Memory operations
            memory_ops(int_arr, float_arr, double_arr, v10 + inner);
            
            // Conditional execution paths
            switch (inner % 4) {
                case 0:
                    // FP math branch
                    f5 = f6 * f7 - f8 / f9;
                    d2 = d3 * 1.1 + d4 * 0.9;
                    v10 = (int)(f5 * 100.0f) ^ (int)(d2 * 50.0);
                    break;
                case 1:
                    // Integer bit manipulation branch
                    v11 = (v12 << 3) | (v13 >> 2);
                    v14 = v15 ^ v16 & v17;
                    v18 = ~v19 + v20;
                    break;
                case 2:
                    // Mixed operations
                    v2 = v3 * v4 + v5;
                    f6 = (float)v2 * 0.25f;
                    d3 = (double)f6 * 2.0;
                    int_arr[(v2 + inner) % 64] = (int)d3;
                    break;
                case 3:
                    // Memory intensive
                    for (int k = 0; k < 8; k++) {
                        int idx = (inner + k) % 64;
                        int_arr[idx] = int_arr[(idx + 1) % 64] + 
                                      int_arr[(idx + 31) % 64];
                        asm volatile("" : : : "memory");
                    }
                    break;
            }
            
            // Update many variables to keep them live
            v12 = v11 + v10;
            v13 = v12 * 2 - v11;
            f7 = f6 * 1.1f + f5 * 0.9f;
            f8 = f7 / 2.0f - f6;
            d4 = d3 * 0.8 + d2 * 0.2;
            d5 = d4 * 1.5 - d3;
            
            asm volatile("" : : : "memory");
        }
        
        // Deeply nested third loop
        volatile int deep_limit = (outer % 3) + 2;
        for (volatile int deep = 0; deep < deep_limit; deep++) {
            // Complex dependency across iterations
            v15 = v16 + v17 - v18;
            f9 = (float)v15 * 0.33f;
            v16 = (int)(f9 * 10.0f) ^ v19;
            f10 = f9 * 2.0f - f8;
            v17 = (int)f10 + v20;
            
            // Another helper call
            v18 = helper_func1(v17, v19, f10, d5);
            
            asm volatile("" : : : "memory");
        }
        
        // Accumulate to checksum
        checksum ^= v1 ^ v6 ^ v11 ^ v16 ^ (int)f3 ^ (int)d1;
    }
    
    // Final computation using all variables
    int final_result = checksum;
    final_result ^= v2 ^ v3 ^ v4 ^ v5 ^ v7 ^ v8 ^ v9 ^ v10;
    final_result ^= v12 ^ v13 ^ v14 ^ v15 ^ v17 ^ v18 ^ v19 ^ v20;
    final_result ^= (int)f1 ^ (int)f2 ^ (int)f4 ^ (int)f5;
    final_result ^= (int)f6 ^ (int)f7 ^ (int)f8 ^ (int)f9 ^ (int)f10;
    final_result ^= (int)d2 ^ (int)d3 ^ (int)d4 ^ (int)d5;
    
    // Use array elements
    for (int i = 0; i < 16; i++) {
        final_result ^= int_arr[i] ^ (int)float_arr[i % 32] ^ (int)double_arr[i];
    }
    
    printf("Final checksum: %d\n", final_result);
    
    return final_result & 0xFF;
}

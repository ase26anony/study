#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define BARRIER() asm volatile("" ::: "memory")

// Helper functions that won't be inlined
NOINLINE float helper_float(float a, float b, float c) {
    BARRIER();
    float r = (a * b) + (c / 2.0f);
    BARRIER();
    return r * 0.75f;
}

NOINLINE int helper_int(int a, int b, int c) {
    BARRIER();
    int r = (a ^ b) | (c & 0x7FFFFFFF);
    BARRIER();
    return r + (b >> 3);
}

NOINLINE double helper_double(double a, double b, int c) {
    BARRIER();
    double r = a * b - (double)c;
    BARRIER();
    return r / 3.14159;
}

NOINLINE void helper_memory(int* arr, float* farr, int idx) {
    BARRIER();
    arr[idx] = (int)(farr[idx] * 100.0f);
    BARRIER();
    farr[idx] = (float)arr[idx] / 50.0f;
    BARRIER();
}

int main(void) {
    // High register pressure: many local variables of different types
    volatile int outer_limit = 1000;  // Prevent constant propagation
    volatile int inner1_base = 50;
    volatile int inner2_base = 20;
    
    // Integer variables
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    // Floating point variables
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    float f11 = 11.11f, f12 = 12.12f, f13 = 13.13f, f14 = 14.14f, f15 = 15.15f;
    
    // Double variables
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    // Arrays for memory operations
    int int_array[64];
    float float_array[64];
    
    // Initialize arrays
    for (int i = 0; i < 64; i++) {
        int_array[i] = i * 3;
        float_array[i] = (float)i * 1.5f;
    }
    
    // Final checksum
    volatile uint64_t checksum = 0;
    
    // Outer loop with volatile limit
    for (int outer = 0; outer < outer_limit; outer++) {
        BARRIER();
        
        // Nested loop 1: variable bound based on outer index
        int inner1_limit = inner1_base + (outer % 10);
        for (int i1 = 0; i1 < inner1_limit; i1++) {
            // Mixed operation dependency chain
            v1 = v2 + v3 * i1;
            f1 = (float)v1 / 3.14159f;
            BARRIER();
            
            d1 = (double)f1 * 2.71828;
            v4 = (int)d1 ^ v5;
            BARRIER();
            
            // Memory access creating dependencies
            int_array[i1 % 64] = v4;
            f2 = float_array[(i1 + 1) % 64];
            BARRIER();
            
            // Conditional execution paths
            switch (i1 % 4) {
                case 0:
                    // FP math path
                    f3 = helper_float(f1, f2, (float)v1);
                    d2 = helper_double(d1, (double)f3, v4);
                    v6 = (int)(d2 * 100.0);
                    break;
                case 1:
                    // Integer bit manipulation path
                    v7 = helper_int(v1, v4, v6);
                    v8 = (v7 << 3) | (v7 >> 29);
                    v9 = v8 ^ 0xAAAAAAAA;
                    break;
                case 2:
                    // Mixed path with memory
                    helper_memory(int_array, float_array, i1 % 64);
                    f4 = float_array[i1 % 64] * 2.0f;
                    v10 = int_array[(i1 + 2) % 64] + (int)f4;
                    break;
                default:
                    // Complex dependency chain
                    v11 = v1 * v4 - v6;
                    f5 = helper_float((float)v11, f1, f2);
                    d3 = d1 * (double)f5;
                    v12 = helper_int((int)d3, v7, v8);
                    break;
            }
            BARRIER();
            
            // More operations to increase pressure
            v13 = v1 + v4;
            f6 = f1 * f3;
            d4 = d2 / d1;
            v14 = v6 | v7;
            f7 = f4 - f5;
            
            // Nested loop 2: deeper nesting
            int inner2_limit = inner2_base + (i1 % 5);
            for (int i2 = 0; i2 < inner2_limit; i2++) {
                BARRIER();
                // Interleaved operations
                v15 = v13 * i2 + v14;
                f8 = (float)v15 / (float)(i2 + 1);
                BARRIER();
                
                d5 = (double)f8 * d4;
                int_array[(i1 + i2) % 64] = (int)d5;
                BARRIER();
                
                // Function call with dependencies
                v16 = helper_int(v15, int_array[i2 % 64], outer);
                f9 = helper_float(f8, f6, (float)v16);
                BARRIER();
                
                // Update checksum
                checksum ^= (uint64_t)v16;
                checksum += (uint64_t)(f9 * 1000.0f);
            }
            
            // Update variables for next iteration
            v2 = v13 ^ v15;
            v3 = v14 + v16;
            f2 = f6 * f9;
            d1 = d4 + d5;
        }
        
        // Additional operations between outer loop iterations
        v17 = v1 * v2 - v3 * v4;
        f10 = helper_float(f1, f2, f3);
        BARRIER();
        
        v18 = helper_int(v17, v5, v6);
        d2 = helper_double(d1, (double)f10, v18);
        BARRIER();
        
        // Memory operations spanning iterations
        for (int i = 0; i < 8; i++) {
            int idx = (outer + i) % 64;
            float_array[idx] = (float)int_array[idx] * 0.25f;
            int_array[idx] = (int)(float_array[idx] * 4.0f);
        }
        
        // Update checksum with outer loop results
        checksum ^= (uint64_t)v17;
        checksum ^= (uint64_t)(f10 * 100.0f);
        checksum += (uint64_t)d2;
    }
    
    // Final complex calculation
    v19 = 0;
    f11 = 0.0f;
    for (int i = 0; i < 64; i++) {
        v19 += int_array[i];
        f11 += float_array[i];
        BARRIER();
    }
    
    v20 = helper_int(v19, (int)f11, (int)checksum);
    f12 = helper_float(f11, (float)v19, (float)(checksum & 0xFFFFFFFF));
    
    // Final checksum output
    checksum ^= (uint64_t)v20;
    checksum ^= (uint64_t)(f12 * 10000.0f);
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// Helper functions that won't be inlined
__attribute__((noinline)) float helper_float_op(float a, float b, float c) {
    volatile float v1 = a * b;
    float v2 = v1 + c;
    asm volatile("" ::: "memory");
    return v2 * 0.5f - v1;
}

__attribute__((noinline)) int helper_int_op(int a, int b, int c) {
    volatile int v1 = a ^ b;
    int v2 = (v1 << 3) | (v1 >> 29);
    asm volatile("" ::: "memory");
    return (v2 + c) * 7;
}

__attribute__((noinline)) double helper_mixed_op(int a, float b, double c) {
    volatile double d1 = (double)a * (double)b;
    double d2 = d1 + c;
    asm volatile("" ::: "memory");
    return d2 * 1.5 - d1 / 2.0;
}

// Complex function with high register pressure and mixed operations
__attribute__((noinline)) uint64_t complex_scheduling_function(volatile int outer_limit) {
    // Declare many variables to create high register pressure
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    int i11 = 11, i12 = 12, i13 = 13, i14 = 14, i15 = 15;
    float f11 = 11.11f, f12 = 12.12f, f13 = 13.13f, f14 = 14.14f, f15 = 15.15f;
    double d11 = 11.011, d12 = 12.012, d13 = 13.013, d14 = 14.014, d15 = 15.015;
    int i16 = 16, i17 = 17, i18 = 18, i19 = 19, i20 = 20;
    float f16 = 16.16f, f17 = 17.17f, f18 = 18.18f, f19 = 19.19f, f20 = 20.20f;
    
    // Local arrays to create memory pressure
    int arr_int[32];
    float arr_float[32];
    double arr_double[32];
    
    volatile uint64_t checksum = 0;
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        // Initialize arrays with pattern
        for (int i = 0; i < 32; i++) {
            arr_int[i] = i + outer;
            arr_float[i] = (float)(i * 1.1) + outer;
            arr_double[i] = (double)(i * 1.01) + outer;
        }
        
        // Middle loop with variable bound
        volatile int middle_limit = (outer % 8) + 3;
        for (int middle = 0; middle < middle_limit; middle++) {
            // Inner loop with complex dependency chain
            volatile int inner_limit = (middle * 7 + outer * 3) % 16 + 4;
            for (int inner = 0; inner < inner_limit; inner++) {
                // Mixed operation dependency chain
                // int -> float -> double -> memory -> int
                i6 = v1 + v2 * inner;
                f6 = (float)i6 * f1;
                d6 = (double)f6 * d1;
                
                // Memory store with barrier
                arr_int[inner % 32] = i6;
                asm volatile("" ::: "memory");
                
                // Load and continue chain
                i7 = arr_int[(inner + 1) % 32];
                f7 = helper_float_op(f6, f2, (float)i7);
                
                // Another barrier
                asm volatile("" ::: "memory");
                
                // More mixed operations
                d7 = helper_mixed_op(i7, f7, d6);
                i8 = helper_int_op(i6, i7, (int)d7);
                
                // Conditional execution paths
                switch (inner % 5) {
                    case 0:
                        // FP math path
                        f8 = f7 * f3 - f6 / f4;
                        d8 = sqrt(fabs(d7 * d2 - d6));
                        i9 = (int)(f8 * d8);
                        break;
                    case 1:
                        // Integer bit manipulation path
                        i9 = (i8 << 4) ^ (i7 >> 2);
                        i9 = (i9 * 13) & 0xFFFF;
                        f8 = (float)(i9 ^ 0xAAAA);
                        break;
                    case 2:
                        // Memory intensive path
                        for (int j = 0; j < 4; j++) {
                            arr_float[(inner + j) % 32] = f7 * j;
                            arr_double[(inner + j) % 32] = d7 / (j + 1);
                        }
                        i9 = arr_int[inner % 32] + arr_int[(inner + 1) % 32];
                        f8 = arr_float[inner % 32];
                        break;
                    case 3:
                        // Function call intensive path
                        i9 = helper_int_op(i8, inner, outer);
                        f8 = helper_float_op(f7, (float)i9, f4);
                        d8 = helper_mixed_op(i9, f8, d7);
                        i9 = (int)d8;
                        break;
                    default:
                        // Mixed operations
                        i9 = i8 * 3 - i7 / 2;
                        f8 = f7 * 1.5f + f6 * 0.5f;
                        d8 = d7 * 2.0 - d6;
                        break;
                }
                
                // Update checksum with all live variables
                checksum ^= (uint64_t)i6;
                checksum ^= (uint64_t)(*(uint32_t*)&f7);
                checksum ^= (uint64_t)(*(uint64_t*)&d8);
                checksum ^= (uint64_t)i9;
                
                // More operations to extend dependency chain
                v1 = v1 ^ i9;
                f1 = f1 + f8 * 0.1f;
                d1 = d1 * 0.99 + d8 * 0.01;
                
                // Another memory barrier
                asm volatile("" ::: "memory");
                
                // Additional integer operations
                i10 = i9 * 11 - i8 * 7 + i7 * 3;
                i11 = (i10 & 0xFF) | ((i10 & 0xFF00) >> 8) << 16;
                i12 = i11 ^ 0x12345678;
                
                // Update array elements
                arr_int[(inner + 5) % 32] = i12;
                arr_float[(inner + 3) % 32] = f8;
                arr_double[(inner + 7) % 32] = d8;
            }
            
            // Inter-loop operations
            v2 = v2 + middle * 17;
            f2 = f2 * 1.1f - (float)middle * 0.01f;
            d2 = d2 / 1.5 + (double)middle * 0.001;
            
            // Call helper function across loop boundary
            if (middle % 2 == 0) {
                i13 = helper_int_op(v2, i12, middle);
                f9 = helper_float_op(f2, (float)i13, 3.14f);
            }
        }
        
        // Update volatile variables to prevent optimization
        v3 = v3 ^ outer;
        f3 = f3 + (float)outer * 0.001f;
        d3 = d3 * (1.0 + outer * 0.0001);
        
        // Complex final calculation per outer iteration
        int temp_sum = 0;
        float temp_float = 0.0f;
        double temp_double = 0.0;
        
        for (int k = 0; k < 16; k++) {
            temp_sum += arr_int[k];
            temp_float += arr_float[k];
            temp_double += arr_double[k];
        }
        
        checksum ^= (uint64_t)temp_sum;
        checksum ^= (uint64_t)(*(uint32_t*)&temp_float);
        checksum ^= (uint64_t)(*(uint64_t*)&temp_double);
    }
    
    return checksum;
}

int main() {
    // Volatile outer limit to prevent constant propagation
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling stress test...\n");
    
    // Run the complex function
    uint64_t result = complex_scheduling_function(iterations);
    
    // Print result to prevent dead code elimination
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    
    // Additional volatile operations to ensure all code paths are used
    volatile int verify = (result != 0) ? 1 : 0;
    if (verify) {
        printf("Test completed successfully.\n");
    }
    
    return 0;
}

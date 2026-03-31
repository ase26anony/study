#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define NOINLINE __attribute__((noinline))

// Helper functions that won't be inlined
NOINLINE float helper_float_op(float a, float b, float c) {
    volatile float v1 = a * b;
    volatile float v2 = c / 3.14159f;
    asm volatile("" ::: "memory");
    return v1 + v2 - (a * 0.5f);
}

NOINLINE int helper_int_op(int a, int b, int c) {
    volatile int v1 = a ^ b;
    volatile int v2 = c << 3;
    asm volatile("" ::: "memory");
    return (v1 | v2) & 0x7FFFFFFF;
}

NOINLINE double helper_double_op(double a, double b) {
    volatile double v1 = sin(a) * cos(b);
    volatile double v2 = sqrt(fabs(a) + 1.0);
    asm volatile("" ::: "memory");
    return v1 * v2 + a - b;
}

NOINLINE void memory_barrier_helper(int* ptr, float* fptr, double* dptr) {
    volatile int temp = *ptr;
    volatile float ftemp = *fptr;
    volatile double dtemp = *dptr;
    asm volatile("" ::: "memory");
    *ptr = temp + 1;
    *fptr = ftemp * 1.1f;
    *dptr = dtemp * 0.99;
}

// Main complex function with high register pressure
NOINLINE uint64_t complex_scheduling_function(volatile int outer_limit) {
    // Declare many variables to create register pressure (30+)
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    volatile int arr_idx1 = 0, arr_idx2 = 0, arr_idx3 = 0;
    volatile float farr_idx1 = 0.0f, farr_idx2 = 0.0f;
    volatile double darr_idx1 = 0.0, darr_idx2 = 0.0;
    
    // Additional variables for more pressure
    int extra1 = 100, extra2 = 200, extra3 = 300, extra4 = 400;
    float fextra1 = 100.5f, fextra2 = 200.5f, fextra3 = 300.5f;
    double dextra1 = 1000.5, dextra2 = 2000.5, dextra3 = 3000.5;
    
    // Arrays to create memory dependencies
    int int_array[64];
    float float_array[64];
    double double_array[64];
    
    // Initialize arrays
    for (int i = 0; i < 64; i++) {
        int_array[i] = i;
        float_array[i] = i * 1.5f;
        double_array[i] = i * 2.5;
    }
    
    uint64_t checksum = 0;
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        // Nested loop level 1 - variable bound based on outer
        volatile int middle_limit = (outer % 8) + 3;
        for (volatile int middle = 0; middle < middle_limit; middle++) {
            // Nested loop level 2 - variable bound based on middle
            volatile int inner_limit = (middle % 4) + 2;
            for (volatile int inner = 0; inner < inner_limit; inner++) {
                // Complex conditional execution paths
                switch ((outer + middle + inner) % 5) {
                    case 0: {
                        // FP math intensive path
                        f1 = helper_float_op(f1, f2, f3);
                        f4 = f5 * f6 + f7 / f8;
                        d1 = helper_double_op(d1, d2);
                        d3 = sin(d4) * cos(d5);
                        
                        // Memory access pattern
                        arr_idx1 = (arr_idx1 + 1) & 63;
                        farr_idx1 = fmodf(farr_idx1 + 1.7f, 64.0f);
                        float_array[(int)farr_idx1] = f1 * f2;
                        int_array[arr_idx1] = (int)(f1 * 100.0f);
                        
                        // Barrier
                        asm volatile("" ::: "memory");
                        
                        // More operations
                        f2 = helper_float_op(f3, f4, f5);
                        d2 = d3 * d4 - d5 / d6;
                        break;
                    }
                    
                    case 1: {
                        // Integer intensive path
                        v1 = helper_int_op(v1, v2, v3);
                        v4 = (v5 ^ v6) | (v7 & v8);
                        v9 = (v10 << 2) | (v1 >> 3);
                        
                        // Memory access with barrier
                        arr_idx2 = (arr_idx2 + 3) & 63;
                        int_array[arr_idx2] = v1 + v2 + v3;
                        asm volatile("" ::: "memory");
                        
                        // More integer ops
                        v2 = helper_int_op(v4, v5, v6);
                        v3 = (v7 * v8) ^ (v9 + v10);
                        break;
                    }
                    
                    case 2: {
                        // Mixed operations path
                        f3 = helper_float_op(f4, f5, f6);
                        v5 = helper_int_op(v6, v7, v8);
                        d4 = helper_double_op(d5, d6);
                        
                        // Complex dependency chain
                        int temp_int = (int)(f3 * 100.0f);
                        v6 = v5 ^ temp_int;
                        f4 = (float)v6 / 123.45f;
                        d5 = (double)temp_int * 0.01234;
                        
                        // Memory barrier and function call
                        asm volatile("" ::: "memory");
                        memory_barrier_helper(&int_array[arr_idx1], 
                                            &float_array[(int)farr_idx1],
                                            &double_array[arr_idx2]);
                        break;
                    }
                    
                    case 3: {
                        // Memory intensive path
                        for (volatile int mem_iter = 0; mem_iter < 3; mem_iter++) {
                            arr_idx3 = (arr_idx3 + mem_iter) & 63;
                            farr_idx2 = fmodf(farr_idx2 + 2.3f, 64.0f);
                            darr_idx1 = fmod(darr_idx1 + 1.23, 64.0);
                            
                            int_array[arr_idx3] += v1 + v2;
                            float_array[(int)farr_idx2] *= f1 + f2;
                            double_array[(int)darr_idx1] = d1 * d2 - d3;
                            
                            asm volatile("" ::: "memory");
                        }
                        break;
                    }
                    
                    case 4: {
                        // Another mixed path with function calls
                        float f_result = helper_float_op(f7, f8, f9);
                        int i_result = helper_int_op(v9, v10, v1);
                        double d_result = helper_double_op(d7, d8);
                        
                        // Create cross-type dependencies
                        v7 = (int)(f_result * 1000.0f) ^ i_result;
                        f8 = (float)i_result / 456.78f + f_result;
                        d8 = d_result * (double)v7 * 0.001;
                        
                        // Memory operations
                        arr_idx1 = (arr_idx1 + i_result) & 63;
                        int_array[arr_idx1] = v7;
                        float_array[arr_idx1] = f8;
                        
                        asm volatile("" ::: "memory");
                        break;
                    }
                }
                
                // Additional operations outside switch
                v10 = v1 + v2 - v3 * v4;
                f10 = f1 * f2 - f3 / f4 + f5;
                d10 = d1 * d2 + sin(d3) * cos(d4);
                
                // Update checksum with various values
                checksum ^= (uint64_t)v1;
                checksum ^= (uint64_t)(f1 * 1000.0f);
                checksum ^= (uint64_t)(d1 * 10000.0);
                checksum ^= (uint64_t)int_array[arr_idx1];
            }
            
            // Middle loop operations
            f5 = helper_float_op(f6, f7, f8);
            v8 = helper_int_op(v9, v10, v1);
            asm volatile("" ::: "memory");
        }
        
        // Outer loop operations with function call
        d6 = helper_double_op(d7, d8);
        memory_barrier_helper(&extra1, &fextra1, &dextra1);
        
        // Update array elements
        for (volatile int update = 0; update < 2; update++) {
            int idx = (outer + update) & 63;
            int_array[idx] += v1 + v2 + v3;
            float_array[idx] = float_array[idx] * 1.01f + f1;
            double_array[idx] = double_array[idx] * 0.99 + d1;
        }
        
        asm volatile("" ::: "memory");
    }
    
    // Final accumulation from arrays
    for (int i = 0; i < 64; i++) {
        checksum ^= (uint64_t)int_array[i];
        checksum ^= (uint64_t)(float_array[i] * 100.0f);
        checksum ^= (uint64_t)(double_array[i] * 1000.0);
    }
    
    // Include all local variables in final checksum
    checksum ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    checksum ^= (uint64_t)(f1 * 100.0f) ^ (uint64_t)(f2 * 100.0f);
    checksum ^= (uint64_t)(f3 * 100.0f) ^ (uint64_t)(f4 * 100.0f);
    checksum ^= (uint64_t)(f5 * 100.0f) ^ (uint64_t)(f6 * 100.0f);
    checksum ^= (uint64_t)(f7 * 100.0f) ^ (uint64_t)(f8 * 100.0f);
    checksum ^= (uint64_t)(f9 * 100.0f) ^ (uint64_t)(f10 * 100.0f);
    checksum ^= (uint64_t)(d1 * 1000.0) ^ (uint64_t)(d2 * 1000.0);
    checksum ^= (uint64_t)(d3 * 1000.0) ^ (uint64_t)(d4 * 1000.0);
    checksum ^= (uint64_t)(d5 * 1000.0) ^ (uint64_t)(d6 * 1000.0);
    checksum ^= (uint64_t)(d7 * 1000.0) ^ (uint64_t)(d8 * 1000.0);
    checksum ^= (uint64_t)(d9 * 1000.0) ^ (uint64_t)(d10 * 1000.0);
    
    return checksum;
}

int main() {
    // Volatile to prevent optimization
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling stress test...\n");
    
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}

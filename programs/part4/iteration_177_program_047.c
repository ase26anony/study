#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Non-inline helper functions to force scheduler state saves/restores
__attribute__((noinline)) 
float helper_float_op(float a, float b, float c) {
    volatile float v1 = a * b;
    volatile float v2 = b + c;
    asm volatile("" ::: "memory");
    return v1 / (v2 + 1.0f);
}

__attribute__((noinline))
int helper_int_op(int a, int b, int c) {
    volatile int v1 = a ^ b;
    volatile int v2 = b | c;
    asm volatile("" ::: "memory");
    return (v1 & v2) * 3;
}

__attribute__((noinline))
double helper_double_op(double a, double b, int c) {
    volatile double v1 = a + b;
    volatile double v2 = b - a;
    asm volatile("" ::: "memory");
    return (v1 * v2) / (c + 1);
}

__attribute__((noinline))
void memory_barrier_helper(int* ptr, float* fptr, double* dptr) {
    volatile int temp = *ptr;
    volatile float ftemp = *fptr;
    volatile double dtemp = *dptr;
    asm volatile("" ::: "memory");
    *ptr = temp + 1;
    *fptr = ftemp * 1.1f;
    *dptr = dtemp * 1.01;
}

// Main complex function with high register pressure
void complex_scheduling_function(volatile int outer_iterations) {
    // Declare many variables to create high register pressure
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    volatile int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    
    // Arrays for memory operations
    int arr_int[64];
    float arr_float[64];
    double arr_double[64];
    
    // Initialize arrays
    for (int i = 0; i < 64; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 1.5f;
        arr_double[i] = i * 2.5;
    }
    
    volatile int checksum = 0;
    volatile float fchecksum = 0.0f;
    volatile double dchecksum = 0.0;
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        // Nested loop with variable bounds
        volatile int inner_limit = (outer % 16) + 8;
        
        for (volatile int mid = 0; mid < inner_limit; mid++) {
            // Innermost loop with data-dependent trip count
            int inner_trip = (mid * v1 + v2) % 32 + 4;
            
            for (int inner = 0; inner < inner_trip; inner++) {
                // Complex mixed operation dependency chain
                // Integer operations
                v1 = v2 + v3;
                v2 = v3 ^ v4;
                v3 = v4 * v5;
                v4 = v5 | v6;
                v5 = v6 & v7;
                
                asm volatile("" ::: "memory");
                
                // Floating point operations using integer results
                f1 = (float)v1 * f2;
                f2 = f1 + (float)v2;
                f3 = f2 / (float)(v3 + 1);
                f4 = f3 - f1;
                
                asm volatile("" ::: "memory");
                
                // Double precision operations
                d1 = (double)f1 * d2;
                d2 = d1 + (double)f2;
                d3 = d2 / (double)(v4 + 1);
                d4 = d3 - d1;
                
                asm volatile("" ::: "memory");
                
                // Memory operations with mixed types
                int idx = (v5 + inner) & 63;
                arr_int[idx] = v1 + v2 + v3;
                arr_float[idx] = f1 + f2 + f3;
                arr_double[idx] = d1 + d2 + d3;
                
                // Load and use results
                v6 = arr_int[(idx + 1) & 63];
                f5 = arr_float[(idx + 2) & 63];
                d5 = arr_double[(idx + 3) & 63];
                
                asm volatile("" ::: "memory");
                
                // Conditional execution paths
                switch (inner & 7) {
                    case 0:
                        // FP math branch
                        f6 = helper_float_op(f1, f2, f3);
                        f7 = f6 * f4 + f5;
                        v7 = (int)f7 * v6;
                        break;
                    case 1:
                        // Integer bit manipulation branch
                        v7 = helper_int_op(v1, v2, v3);
                        v8 = (v7 << 3) | (v6 >> 2);
                        v9 = v8 ^ 0xAAAAAAAA;
                        break;
                    case 2:
                        // Double precision branch
                        d6 = helper_double_op(d1, d2, v4);
                        d7 = d6 * d3 - d4;
                        f8 = (float)d7 * 2.0f;
                        break;
                    case 3:
                        // Memory intensive branch
                        memory_barrier_helper(&arr_int[idx], &arr_float[idx], &arr_double[idx]);
                        v10 = arr_int[idx] + arr_int[(idx + 4) & 63];
                        f9 = arr_float[idx] * arr_float[(idx + 5) & 63];
                        d8 = arr_double[idx] / (arr_double[(idx + 6) & 63] + 1.0);
                        break;
                    case 4:
                        // Mixed operations branch
                        v11 = v7 * v8 + v9;
                        f10 = helper_float_op((float)v11, f3, f4);
                        d9 = helper_double_op((double)f10, d5, v10);
                        v12 = (int)d9;
                        break;
                    default:
                        // Default computation branch
                        v13 = v10 ^ v11 ^ v12;
                        f1 = f10 * 0.5f + f9;
                        d10 = d9 * 0.25 + d8;
                        break;
                }
                
                asm volatile("" ::: "memory");
                
                // More dependency chains
                v14 = v7 + v8 - v9;
                f2 = f6 * f7 / (f8 + 1.0f);
                d1 = d6 + d7 - d8;
                
                v15 = v10 * v11 / (v12 + 1);
                f3 = f9 + f10;
                d2 = d9 * d10;
                
                // Update checksums
                checksum ^= v1 ^ v2 ^ v3 ^ v4 ^ v5;
                checksum += v6 + v7 + v8 + v9 + v10;
                checksum ^= v11 ^ v12 ^ v13 ^ v14 ^ v15;
                
                fchecksum += f1 + f2 + f3 + f4 + f5;
                fchecksum *= 1.0001f;
                
                dchecksum += d1 + d2 + d3 + d4 + d5;
                dchecksum *= 1.0000001;
                
                asm volatile("" ::: "memory");
            }
            
            // Call helper functions between loop iterations
            if (mid & 1) {
                v16 = helper_int_op(v14, v15, checksum & 0xFF);
                f4 = helper_float_op(f3, fchecksum, 2.0f);
                d3 = helper_double_op(d2, dchecksum, v16);
            }
        }
        
        // Additional complex operations between outer iterations
        for (int i = 0; i < 8; i++) {
            int idx = (outer + i) & 63;
            arr_int[idx] = helper_int_op(arr_int[idx], checksum, i);
            arr_float[idx] = helper_float_op(arr_float[idx], fchecksum, (float)i);
            arr_double[idx] = helper_double_op(arr_double[idx], dchecksum, i);
            
            // Memory barrier
            asm volatile("" ::: "memory");
            
            // Use the results
            v17 += arr_int[idx];
            f5 += arr_float[idx];
            d4 += arr_double[idx];
        }
    }
    
    // Final computation to use all variables and prevent dead code elimination
    volatile int final_result = 0;
    final_result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    final_result += v11 + v12 + v13 + v14 + v15 + v16 + v17;
    final_result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    final_result += (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10;
    final_result += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    final_result += (int)d6 + (int)d7 + (int)d8 + (int)d9 + (int)d10;
    
    // Use array elements
    for (int i = 0; i < 64; i += 8) {
        final_result ^= arr_int[i];
        final_result += (int)arr_float[i];
        final_result ^= (int)arr_double[i];
    }
    
    // Print results to ensure execution
    printf("Checksums: int=%d, float=%.2f, double=%.2f, final=%d\n", 
           checksum, fchecksum, dchecksum, final_result);
}

int main() {
    // Volatile to prevent constant propagation
    volatile int iterations = 100;
    
    printf("Starting complex scheduling test...\n");
    complex_scheduling_function(iterations);
    printf("Test completed.\n");
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define NUM_VARS 35
#define OUTER_ITER 1000
#define INNER_ITER 50

// Non-inline helper functions to force scheduler state saves
__attribute__((noinline)) 
float helper_float_ops(float a, float b, float c, int count) {
    volatile float result = a;
    for (int i = 0; i < count; i++) {
        result = result * b + c;
        result = result / (b + 1.0f);
        asm volatile("" ::: "memory");  // Memory barrier
    }
    return result;
}

__attribute__((noinline))
int helper_int_ops(int a, int b, int c, volatile int* counter) {
    int result = a ^ b;
    for (int i = 0; i < (*counter % 8) + 1; i++) {
        result = (result << 3) | (result >> 29);
        result = result + c * i;
        result = result ^ (b << i);
        asm volatile("" ::: "memory");
    }
    return result;
}

__attribute__((noinline))
double helper_mixed_ops(double* arr, int idx, float fval) {
    double sum = 0.0;
    for (int i = idx % 4; i < 8; i++) {
        sum += arr[i] * fval;
        sum = sqrt(fabs(sum));
        asm volatile("" ::: "memory");
    }
    return sum;
}

__attribute__((noinline))
void helper_memory_ops(int* dest, const int* src, int size, volatile int* modifier) {
    for (int i = 0; i < size; i++) {
        dest[i] = src[i] + *modifier;
        // Create dependency chain
        if (i > 0) {
            dest[i] ^= dest[i-1];
        }
        asm volatile("" ::: "memory");
    }
}

int main() {
    // Many local variables to create register pressure
    volatile int outer_counter = OUTER_ITER;  // Prevent constant propagation
    volatile int inner_mod = 7;
    
    // Integer variables
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    // Floating point variables
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    // Double precision variables
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    
    // Additional variables for more pressure
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    float f11 = 11.11f, f12 = 12.12f, f13 = 13.13f;
    double d11 = 11.11, d12 = 12.12;
    
    // Arrays for memory operations
    int arr_int[16];
    float arr_float[16];
    double arr_double[16];
    
    // Initialize arrays
    for (int i = 0; i < 16; i++) {
        arr_int[i] = i * 3;
        arr_float[i] = i * 1.5f;
        arr_double[i] = i * 2.5;
    }
    
    // Final checksum accumulator
    volatile uint64_t checksum = 0;
    
    // Outer loop with volatile limit
    for (int outer = 0; outer < outer_counter; outer++) {
        // Nested loop with variable bounds
        volatile int inner_limit = INNER_ITER + (outer % 10);
        
        // First inner loop - mixed operations
        for (int i = 0; i < inner_limit; i++) {
            // Complex dependency chain: int -> float -> double -> memory
            v1 = v1 * v2 + v3;
            f1 = (float)v1 * f2;
            d1 = (double)f1 * d2;
            arr_double[i % 8] = d1;
            
            // Memory barrier to prevent reordering
            asm volatile("" ::: "memory");
            
            // Load and continue chain
            v4 = (int)arr_double[(i + 1) % 8] + v5;
            f3 = f4 * (float)v4;
            
            // Call helper function with many live variables
            f5 = helper_float_ops(f3, f6, f7, (i % 4) + 1);
            
            // More operations using result
            v6 = (int)f5 + v7 * v8;
            arr_int[i % 8] = v6;
            
            asm volatile("" ::: "memory");
        }
        
        // Second nested loop with different operation mix
        for (int j = 0; j < (inner_limit / 2); j++) {
            // Conditional execution paths
            switch (j % 4) {
                case 0:  // FP math path
                    f8 = f9 * f10 + f11;
                    d3 = sin(d4) * cos(d5);
                    v9 = (int)(f8 * 100.0f);
                    break;
                    
                case 1:  // Integer bit manipulation path
                    v10 = (v10 << 3) | (v10 >> 29);
                    v11 = v10 ^ v9;
                    v12 = ~v11 & 0xFFFF;
                    f9 = (float)v12 / 256.0f;
                    break;
                    
                case 2:  // Memory intensive path
                    helper_memory_ops(arr_int, arr_int, 8, &inner_mod);
                    v13 = arr_int[j % 8];
                    f10 = (float)v13 * 0.01f;
                    break;
                    
                case 3:  // Mixed operations with function call
                    d6 = helper_mixed_ops(arr_double, j, f12);
                    v14 = (int)d6 * v15;
                    f13 = sqrtf(fabsf((float)v14));
                    break;
            }
            
            // Additional operations common to all paths
            v15 = v14 + v13 * (j % 16);
            d7 = d6 * d8 + d9;
            
            // Call integer helper
            v16 = helper_int_ops(v15, v16, v17, &inner_mod);
            
            asm volatile("" ::: "memory");
        }
        
        // Third level of nesting with data-dependent loop count
        int inner_inner = (v16 % 8) + 2;
        for (int k = 0; k < inner_inner; k++) {
            // Interleaved operations creating long dependency chains
            v17 = v16 * v17 + k;
            f11 = f13 * (float)v17;
            d8 = d7 * (double)f11;
            
            v18 = v17 ^ v18;
            f12 = f11 + f12 * 0.5f;
            d9 = d8 / (d9 + 1.0);
            
            // Store to memory
            arr_float[k % 8] = f12;
            arr_double[k % 8] = d9;
            
            // Load with dependency
            v19 = (int)arr_float[(k + 1) % 8];
            v20 = v19 + arr_int[k % 8];
            
            asm volatile("" ::: "memory");
        }
        
        // Update checksum with all variables
        checksum ^= (uint64_t)v1;
        checksum ^= (uint64_t)(f1 * 1000);
        checksum ^= (uint64_t)(d1 * 1000);
        checksum ^= (uint64_t)v20;
        
        // Modify volatile variable to affect next iteration
        inner_mod = (inner_mod * 13 + 17) % 256;
        
        // Additional memory barrier
        asm volatile("" ::: "memory");
    }
    
    // Final computation using all variables
    uint64_t final_result = checksum;
    final_result ^= (uint64_t)(v1 + v2 + v3 + v4 + v5);
    final_result ^= (uint64_t)(v6 + v7 + v8 + v9 + v10);
    final_result ^= (uint64_t)(v11 + v12 + v13 + v14 + v15);
    final_result ^= (uint64_t)(v16 + v17 + v18 + v19 + v20);
    final_result ^= (uint64_t)(f1 * f2 * 1000);
    final_result ^= (uint64_t)(d1 * d2 * 1000);
    
    printf("Final checksum: %llu\n", (unsigned long long)final_result);
    
    return 0;
}

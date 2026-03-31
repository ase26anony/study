#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define NOINLINE __attribute__((noinline))

// Helper functions that won't be inlined
NOINLINE float helper_float_op(float a, float b, float c) {
    volatile float barrier = a + b;
    asm volatile("" ::: "memory");
    return (barrier * c) / (a - b + 1.0f);
}

NOINLINE int helper_int_op(int a, int b, int c) {
    volatile int barrier = a ^ b;
    asm volatile("" ::: "memory");
    return (barrier * c) + (a & b) - (a | b);
}

NOINLINE double helper_double_op(double a, double b, int scale) {
    volatile double barrier = sin(a) * cos(b);
    asm volatile("" ::: "memory");
    return barrier * scale + tan(a + b);
}

NOINLINE void memory_barrier_helper(int* arr, float* farr, double* darr, int idx) {
    volatile int temp = arr[idx];
    asm volatile("" ::: "memory");
    farr[idx] = temp * 0.5f;
    darr[idx] = temp * 0.25;
    arr[idx] = temp + 1;
}

// Main complex function with high register pressure
NOINLINE uint64_t complex_scheduling_function(volatile int outer_iterations) {
    // Many local variables to create register pressure (30+)
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    volatile int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    volatile int v21 = 21, v22 = 22, v23 = 23, v24 = 24, v25 = 25;
    volatile int v26 = 26, v27 = 27, v28 = 28, v29 = 29, v30 = 30;
    
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    float f11 = 11.11f, f12 = 12.12f, f13 = 13.13f, f14 = 14.14f, f15 = 15.15f;
    
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    
    // Arrays for memory operations
    int int_array[64];
    float float_array[64];
    double double_array[64];
    
    // Initialize arrays
    for (int i = 0; i < 64; i++) {
        int_array[i] = i;
        float_array[i] = i * 0.5f;
        double_array[i] = i * 0.25;
    }
    
    volatile uint64_t checksum = 0;
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        // Mixed operation dependency chain
        v1 = v2 + v3;
        f1 = (float)v1 * f2;
        asm volatile("" ::: "memory");  // Barrier
        
        // Nested loops with variable bounds
        volatile int middle_limit = (outer % 8) + 2;
        for (volatile int middle = 0; middle < middle_limit; middle++) {
            volatile int inner_limit = (v1 + middle) % 16 + 1;
            
            for (volatile int inner = 0; inner < inner_limit; inner++) {
                // Complex conditional execution paths
                switch ((inner + outer + middle) % 5) {
                    case 0:  // FP math branch
                        f3 = helper_float_op(f1, f2, f3);
                        d1 = helper_double_op(d1, d2, v1);
                        f4 = sinf(f3) * cosf(f4);
                        d3 = d1 * d2 + d3 / (d4 + 1.0);
                        asm volatile("" ::: "memory");
                        break;
                        
                    case 1:  // Integer bit manipulation branch
                        v4 = helper_int_op(v2, v3, v4);
                        v5 = (v4 << 3) | (v5 >> 2);
                        v6 = v5 ^ v6 ^ v7;
                        v8 = (v8 & 0xFFFF) | ((v9 & 0xFFFF) << 16);
                        asm volatile("" ::: "memory");
                        break;
                        
                    case 2:  // Memory access pattern
                        memory_barrier_helper(int_array, float_array, double_array, 
                                            (inner + outer) % 64);
                        v10 = int_array[(inner * 7) % 64];
                        f5 = float_array[(inner * 11) % 64];
                        d5 = double_array[(inner * 13) % 64];
                        asm volatile("" ::: "memory");
                        break;
                        
                    case 3:  // Mixed type operations
                        v11 = (int)(f5 * 100.0f) + v10;
                        f6 = (float)v11 / 17.0f + f6;
                        d6 = (double)v11 / 23.0 + d6;
                        v12 = (int)(d6 * 1000.0) ^ v12;
                        asm volatile("" ::: "memory");
                        break;
                        
                    case 4:  // Complex dependency chain
                        v13 = v12 + v11 - v10;
                        f7 = helper_float_op(f6, f7, (float)v13);
                        v14 = helper_int_op(v13, v14, (int)f7);
                        d7 = helper_double_op(d6, d7, v14);
                        f8 = (float)d7 * f8 / (f9 + 1.0f);
                        v15 = (int)(f8 * 100.0f) + v15;
                        asm volatile("" ::: "memory");
                        break;
                }
                
                // Cross-type data dependencies
                v16 = v15 + (int)f7;
                f9 = f8 * (float)v16;
                d8 = d7 / (double)v16;
                v17 = v16 ^ (int)(d8 * 1000.0);
                
                // More mixed operations
                f10 = sqrtf(fabsf(f9));
                v18 = (v17 * 1103515245 + 12345) & 0x7fffffff;
                d9 = pow(d8, 1.5);
                v19 = v18 % 100 + v19;
                
                // Memory store with barrier
                int_array[inner % 64] = v19;
                asm volatile("" ::: "memory");
                float_array[inner % 64] = f10;
                double_array[inner % 64] = d9;
            }
            
            // Function call with scheduling side effects
            if (middle % 2 == 0) {
                v20 = helper_int_op(v19, v18, v20);
                f11 = helper_float_op(f10, f9, f11);
            } else {
                d10 = helper_double_op(d9, d8, v20);
                v21 = helper_int_op(v20, v21, (int)d10);
            }
        }
        
        // Accumulate to checksum with mixed operations
        checksum ^= (uint64_t)v1;
        checksum ^= (uint64_t)(f1 * 1000.0f);
        checksum ^= (uint64_t)(d1 * 1000.0);
        checksum += (uint64_t)v15;
        checksum ^= (uint64_t)(f10 * 100.0f);
        checksum += (uint64_t)(d10 * 100.0);
        
        // More operations to keep variables live
        v22 = v21 + v20 + v19;
        f12 = f11 * 1.01f + f10;
        d2 = d1 * 1.01 + d10;
        
        // Additional barriers
        asm volatile("" ::: "memory");
    }
    
    // Final accumulation from all variables
    checksum ^= (uint64_t)v22;
    checksum ^= (uint64_t)(f12 * 100.0f);
    checksum ^= (uint64_t)(d2 * 100.0);
    
    for (int i = 0; i < 32; i++) {
        checksum ^= (uint64_t)int_array[i];
        checksum ^= (uint64_t)(float_array[i] * 100.0f);
        checksum ^= (uint64_t)(double_array[i] * 100.0);
    }
    
    return checksum;
}

int main() {
    // Volatile to prevent constant propagation
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling stress test...\n");
    
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Final checksum: %llu\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}

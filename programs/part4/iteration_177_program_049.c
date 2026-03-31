#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))

// Helper functions that won't be inlined
NOINLINE float helper_float_op(float a, float b, float c) {
    volatile float temp = a * b + c;
    asm volatile("" ::: "memory");
    return temp * 0.5f - b;
}

NOINLINE int helper_int_op(int a, int b, int c) {
    volatile int temp = (a ^ b) | c;
    asm volatile("" ::: "memory");
    return (temp << 3) | (temp >> 29);
}

NOINLINE double helper_double_op(double a, double b) {
    volatile double temp = a / (b + 1.0);
    asm volatile("" ::: "memory");
    return temp * temp - a;
}

// Complex function with high register pressure and mixed operations
NOINLINE uint64_t complex_scheduling_function(volatile int outer_iterations) {
    // Declare many variables to create high register pressure
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    volatile int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    // Arrays for memory operations
    int arr_int[32];
    float arr_float[32];
    double arr_double[32];
    
    // Initialize arrays
    for (int i = 0; i < 32; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 0.5f;
        arr_double[i] = i * 0.25;
    }
    
    uint64_t checksum = 0;
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        // Nested loops with variable bounds
        volatile int middle_limit = (outer % 8) + 3;
        
        for (volatile int middle = 0; middle < middle_limit; middle++) {
            // Inner loop with dependency on outer and middle
            int inner_limit = (outer * 3 + middle * 2) % 16 + 4;
            
            for (int inner = 0; inner < inner_limit; inner++) {
                // Complex conditional execution paths
                switch ((inner + outer + middle) % 5) {
                    case 0: {
                        // FP math path
                        f1 = helper_float_op(f1, f2, f3);
                        f4 = f1 * f2 - f3;
                        asm volatile("" ::: "memory");
                        
                        // Memory operations with FP results
                        arr_float[inner % 32] = f1 + f4;
                        f5 = arr_float[(inner + 1) % 32] * 2.0f;
                        
                        // Integer operations mixed in
                        v1 = v1 ^ v2;
                        v3 = v1 + v2 * v3;
                        break;
                    }
                    case 1: {
                        // Integer bit manipulation path
                        v4 = helper_int_op(v4, v5, v6);
                        v7 = (v4 << v5) | (v6 >> v4);
                        asm volatile("" ::: "memory");
                        
                        // Memory operations with integer results
                        arr_int[inner % 32] = v4 + v7;
                        v8 = arr_int[(inner + 2) % 32] ^ v9;
                        
                        // FP operations mixed in
                        f6 = f6 * 1.1f + f7;
                        break;
                    }
                    case 2: {
                        // Double precision path
                        d1 = helper_double_op(d1, d2);
                        d3 = d1 * d2 - d3;
                        asm volatile("" ::: "memory");
                        
                        // Memory operations with double results
                        arr_double[inner % 32] = d1 + d3;
                        d4 = arr_double[(inner + 3) % 32] / 2.0;
                        
                        // Integer and float mixed
                        v10 = v10 * v11 + v12;
                        f8 = f8 + f9 * 0.5f;
                        break;
                    }
                    case 3: {
                        // Mixed type conversion path
                        float temp_f = (float)v13 * 0.25f;
                        int temp_i = (int)(f10 * 2.0f);
                        asm volatile("" ::: "memory");
                        
                        // Cross-type dependencies
                        v14 = temp_i + v15;
                        f10 = temp_f + f1;
                        
                        // Memory store with type mixing
                        arr_int[(inner + 4) % 32] = (int)(f10 * 100.0f);
                        arr_float[(inner + 5) % 32] = (float)v14 * 0.01f;
                        break;
                    }
                    case 4: {
                        // Complex dependency chain across types
                        v16 = v16 * 3 + v17;
                        f2 = helper_float_op(f2, f3, (float)v16);
                        asm volatile("" ::: "memory");
                        
                        d2 = (double)f2 * 1.5;
                        v18 = helper_int_op(v18, (int)d2, v19);
                        
                        arr_double[inner % 32] = d2;
                        arr_int[(inner + 6) % 32] = v18;
                        
                        f3 = arr_float[(inner + 7) % 32] + f2;
                        break;
                    }
                }
                
                // Additional asm barrier to prevent reordering
                asm volatile("" ::: "memory");
                
                // Update checksum with various values
                checksum ^= (uint64_t)v1;
                checksum ^= (uint64_t)(*(uint32_t*)&f1);
                checksum ^= (uint64_t)(*(uint64_t*)&d1);
                checksum ^= (uint64_t)arr_int[inner % 32];
            }
            
            // Call helper functions between loop levels
            if (middle % 2 == 0) {
                f4 = helper_float_op(f4, f5, f6);
                asm volatile("" ::: "memory");
            } else {
                v5 = helper_int_op(v5, v6, v7);
                asm volatile("" ::: "memory");
            }
        }
        
        // More complex operations between outer iterations
        for (int i = 0; i < 8; i++) {
            // Interleaved memory access pattern
            int idx = (outer + i) % 32;
            arr_int[idx] = arr_int[idx] * 2 + i;
            arr_float[idx] = arr_float[idx] * 1.1f + i;
            arr_double[idx] = arr_double[idx] * 0.9 - i;
            
            // Dependency chain across array operations
            v20 = arr_int[idx] ^ v20;
            f10 = arr_float[idx] + f10;
            d5 = arr_double[idx] - d5;
            
            asm volatile("" ::: "memory");
        }
        
        // Update checksum with more values
        checksum ^= (uint64_t)v20;
        checksum ^= (uint64_t)(*(uint32_t*)&f10);
        checksum ^= (uint64_t)(*(uint64_t*)&d5);
    }
    
    return checksum;
}

int main() {
    // Volatile to prevent constant propagation
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling stress test...\n");
    
    // Call the complex function
    uint64_t result = complex_scheduling_function(iterations);
    
    // Print result to prevent dead code elimination
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    
    return 0;
}

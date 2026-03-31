#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Non-inline helper functions to force scheduler state saves/restores
__attribute__((noinline)) 
float helper_float_op(float a, float b, float c) {
    volatile float barrier = a + b;
    asm volatile("" ::: "memory");
    return (a * b) + (c / (barrier + 1.0f));
}

__attribute__((noinline))
int helper_int_op(int a, int b, int c) {
    volatile int barrier = a ^ b;
    asm volatile("" ::: "memory");
    return (a * b) + (c & barrier);
}

__attribute__((noinline))
double helper_mixed_op(int a, float b, double c) {
    volatile double barrier = (double)a + (double)b;
    asm volatile("" ::: "memory");
    return c * barrier + (double)(a % 256);
}

// Complex function with high register pressure and mixed operations
__attribute__((noinline))
uint64_t complex_scheduling_function(volatile int outer_iterations) {
    // Declare many variables to create high register pressure
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5;
    int *ptr1, *ptr2;
    volatile int mem_barrier;
    
    // Initialize variables
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    v6 = 6; v7 = 7; v8 = 8; v9 = 9; v10 = 10;
    
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    f6 = 6.6f; f7 = 7.7f; f8 = 8.8f; f9 = 9.9f; f10 = 10.10f;
    
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04; d5 = 5.05;
    
    // Allocate some memory to create pointer operations
    ptr1 = (int*)malloc(256 * sizeof(int));
    ptr2 = (int*)malloc(128 * sizeof(int));
    
    // Initialize arrays
    for (int i = 0; i < 256; i++) {
        ptr1[i] = i;
        if (i < 128) ptr2[i] = i * 2;
    }
    
    uint64_t checksum = 0;
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        // Nested loop with variable bounds
        volatile int inner_limit = (outer % 10) + 5;
        
        for (int inner = 0; inner < inner_limit; inner++) {
            // Mixed operation dependency chain
            v1 = v2 + v3;
            asm volatile("" ::: "memory");  // Barrier
            
            f1 = (float)v1 * f2;
            v4 = (int)f1 ^ v5;
            
            // Memory access pattern
            ptr1[v4 % 256] = v1 + v4;
            mem_barrier = ptr1[(v4 + 1) % 256];
            
            f3 = helper_float_op(f1, f2, f3);
            v6 = helper_int_op(v4, v5, v6);
            
            // Conditional execution paths
            switch (inner % 4) {
                case 0:
                    // FP math path
                    d1 = helper_mixed_op(v1, f1, d1);
                    f4 = f3 * f5 + f6;
                    v7 = (int)(f4 * 100.0f);
                    break;
                case 1:
                    // Integer bit manipulation path
                    v7 = (v7 << 3) | (v7 >> 29);
                    v8 = v7 ^ v8;
                    v9 = (v9 * 1103515245 + 12345) & 0x7fffffff;
                    break;
                case 2:
                    // Memory intensive path
                    for (int j = 0; j < 8; j++) {
                        ptr2[j % 128] = ptr1[(v7 + j) % 256] + j;
                    }
                    asm volatile("" ::: "memory");
                    v10 = ptr2[v8 % 128];
                    break;
                case 3:
                    // Mixed type conversions
                    d2 = (double)v7 / (double)(v8 + 1);
                    f5 = (float)d2 * f7;
                    v2 = (int)(f5 * 1000.0f);
                    break;
            }
            
            // Another dependency chain
            f6 = f4 + f5;
            v3 = (int)f6 | v7;
            
            // More memory operations
            ptr1[v3 % 256] = v3 + v8;
            asm volatile("" ::: "memory");
            
            // Deeply nested loop
            volatile int deep_limit = (v3 % 3) + 2;
            for (int deep = 0; deep < deep_limit; deep++) {
                v9 = v9 * 1664525 + 1013904223;
                f7 = f7 * 1.01f + (float)deep * 0.1f;
                d3 = d3 * 1.001 + (double)(v9 % 100) * 0.01;
            }
            
            // Update checksum with various values
            checksum ^= (uint64_t)v1;
            checksum ^= (uint64_t)v3 << 8;
            checksum ^= (uint64_t)v7 << 16;
            checksum ^= (uint64_t)(*(uint32_t*)&f1);
            checksum ^= (uint64_t)(*(uint64_t*)&d1) << 32;
        }
        
        // Additional operations between outer loop iterations
        if (outer % 3 == 0) {
            d4 = helper_mixed_op(v2, f2, d4);
            v5 = helper_int_op(v3, v4, v5);
        } else if (outer % 3 == 1) {
            f8 = helper_float_op(f3, f4, f8);
            v6 = v6 * 13 + 17;
        } else {
            // Complex pointer arithmetic
            for (int i = 0; i < 16; i++) {
                ptr2[i] = ptr1[i * 2] + ptr1[i * 2 + 1];
            }
            asm volatile("" ::: "memory");
        }
        
        // Update floating point values
        f9 = f9 * 0.99f + f8 * 0.01f;
        f10 = f10 * 0.95f + f9 * 0.05f;
        
        d5 = d5 * 0.999 + d4 * 0.001;
    }
    
    // Final accumulation
    for (int i = 0; i < 256; i += 8) {
        checksum ^= (uint64_t)ptr1[i];
    }
    
    for (int i = 0; i < 128; i += 4) {
        checksum ^= (uint64_t)ptr2[i] << 24;
    }
    
    // Cleanup
    free(ptr1);
    free(ptr2);
    
    return checksum;
}

int main() {
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling test...\n");
    
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Checksum result: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}

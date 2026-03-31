#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define NOINLINE __attribute__((noinline))

// Helper functions that won't be inlined
NOINLINE float helper_float_op(float a, float b, float c) {
    volatile float v1 = a * b;
    volatile float v2 = b / (c + 1.0f);
    asm volatile("" ::: "memory");
    return v1 - v2 + sinf(a) * cosf(b);
}

NOINLINE int helper_int_op(int a, int b, int c) {
    volatile int v1 = a ^ b;
    volatile int v2 = b | c;
    volatile int v3 = (a & b) << 3;
    asm volatile("" ::: "memory");
    return (v1 * v2) - (v3 >> 1);
}

NOINLINE double helper_mixed_op(int a, float b, double c) {
    volatile double d1 = (double)a * 1.5;
    volatile double d2 = (double)b * 2.5;
    volatile double d3 = c * 0.75;
    asm volatile("" ::: "memory");
    return d1 * d2 / (d3 + 1e-10);
}

// Complex function with high register pressure and mixed operations
NOINLINE uint64_t complex_scheduling_function(volatile int outer_iterations) {
    // Declare many variables to create high register pressure
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile int arr[32];
    volatile float farr[32];
    volatile double darr[32];
    
    uint64_t checksum = 0;
    
    // Initialize arrays
    for (int i = 0; i < 32; i++) {
        arr[i] = i * 3;
        farr[i] = i * 1.5f;
        darr[i] = i * 2.5;
    }
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        // Nested loops with variable bounds
        volatile int inner_limit = (outer % 8) + 3;
        
        for (volatile int mid = 0; mid < inner_limit; mid++) {
            // Innermost loop with data-dependent trip count
            int inner_count = (v1 + v2 + mid) % 16 + 4;
            
            for (int inner = 0; inner < inner_count; inner++) {
                // Mixed operation dependency chains
                // Integer to float to memory chain
                v1 = v2 * v3 + inner;
                f1 = (float)v1 * f2;
                arr[inner % 32] = (int)f1;
                asm volatile("" ::: "memory");
                
                // Float to double to integer chain
                f3 = f4 * f5 + (float)mid;
                d1 = (double)f3 * d2;
                v4 = (int)d1 ^ arr[(inner + 1) % 32];
                asm volatile("" ::: "memory");
                
                // Memory to float to integer chain
                f6 = farr[inner % 32] * 1.25f;
                v5 = (int)(f6 * 100.0f) | v4;
                darr[inner % 32] = (double)v5 * 0.01;
                asm volatile("" ::: "memory");
                
                // Complex conditional execution paths
                switch (inner % 5) {
                    case 0:
                        // FP math branch
                        f7 = helper_float_op(f1, f2, f3);
                        farr[(inner + outer) % 32] = f7 * 2.0f;
                        v6 = (int)(f7 * 10.0f);
                        break;
                    case 1:
                        // Integer bit manipulation branch
                        v7 = helper_int_op(v1, v2, v3);
                        v7 = (v7 << 2) | (v7 >> 30);
                        arr[(inner + mid) % 32] = v7 ^ 0xABCD;
                        break;
                    case 2:
                        // Mixed operations branch
                        d3 = helper_mixed_op(v4, f4, d1);
                        v8 = (int)(d3 * 1000.0);
                        f8 = (float)v8 / 100.0f;
                        break;
                    case 3:
                        // Memory intensive branch
                        for (int k = 0; k < 4; k++) {
                            arr[(inner + k) % 32] += arr[(inner + k + 1) % 32];
                            farr[(inner + k) % 32] *= 1.1f;
                            darr[(inner + k) % 32] += 0.5;
                        }
                        asm volatile("" ::: "memory");
                        break;
                    case 4:
                        // Computation intensive branch
                        v9 = v1 * v2 - v3 + v4 / (v5 + 1);
                        f9 = sqrtf(fabsf(f1 * f2 - f3 + f4));
                        d4 = pow(d1, 1.5) + log(d2 + 1.0);
                        break;
                }
                
                // Additional volatile barriers
                asm volatile("" ::: "memory");
                
                // Update checksum with various values
                checksum ^= (uint64_t)v1;
                checksum ^= (uint64_t)(*(uint32_t*)&f1);
                checksum ^= (uint64_t)(*(uint64_t*)&d1);
                checksum += (uint64_t)arr[inner % 32];
            }
            
            // Call helper functions across loop iterations
            if (mid % 3 == 0) {
                f2 = helper_float_op(f1, f3, f5);
                asm volatile("" ::: "memory");
            } else if (mid % 3 == 1) {
                v2 = helper_int_op(v3, v4, v5);
                asm volatile("" ::: "memory");
            } else {
                d2 = helper_mixed_op(v6, f6, d3);
                asm volatile("" ::: "memory");
            }
        }
        
        // More complex data flow between outer iterations
        v10 = v1 + v2 + v3 + v4 + v5;
        f10 = f1 + f2 + f3 + f4 + f5;
        
        // Cross-type operations
        for (int i = 0; i < 8; i++) {
            int idx = (outer + i) % 32;
            arr[idx] = (int)(farr[idx] * 10.0f) ^ v10;
            farr[idx] = (float)arr[idx] * 0.1f + f10;
            darr[idx] = (double)arr[idx] + (double)farr[idx];
        }
        asm volatile("" ::: "memory");
    }
    
    // Final aggregation
    for (int i = 0; i < 32; i++) {
        checksum ^= (uint64_t)arr[i];
        checksum += (uint64_t)(*(uint32_t*)&farr[i]);
        checksum ^= (uint64_t)(*(uint64_t*)&darr[i]);
    }
    
    return checksum;
}

int main() {
    // Volatile to prevent constant propagation
    volatile int iterations = 100;
    
    printf("Starting complex scheduling stress test...\n");
    
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}

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
    return v1 + v2 + sinf(a) * cosf(b);
}

NOINLINE int helper_int_op(int a, int b, int c) {
    volatile int v1 = a ^ b;
    volatile int v2 = (b << 3) | (c >> 2);
    asm volatile("" ::: "memory");
    return v1 * v2 + (a & b & c);
}

NOINLINE double helper_mem_op(double* arr, int idx1, int idx2) {
    volatile double v1 = arr[idx1 % 16];
    volatile double v2 = arr[idx2 % 16];
    asm volatile("" ::: "memory");
    arr[(idx1 + idx2) % 16] = v1 * v2 + sqrt(fabs(v1 - v2));
    return arr[(idx1 * idx2) % 16];
}

// Main complex function with high register pressure
NOINLINE uint64_t complex_scheduling_function(volatile int outer_limit) {
    // Many local variables to create register pressure (30+)
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    volatile int counter1 = 0, counter2 = 0, counter3 = 0;
    volatile float accum_f = 0.0f;
    volatile double accum_d = 0.0;
    volatile uint64_t checksum = 0;
    
    // Array for memory operations
    double mem_array[16];
    for (int i = 0; i < 16; i++) {
        mem_array[i] = i * 1.23456789;
    }
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        // Nested loop level 1
        for (volatile int mid = 0; mid < (outer % 7) + 3; mid++) {
            // Nested loop level 2 with volatile bound
            volatile int inner_limit = (mid * outer) % 11 + 2;
            for (volatile int inner = 0; inner < inner_limit; inner++) {
                // Complex dependency chain mixing types
                v1 = v2 + v3 * v4;
                f1 = f2 * f3 + (float)v1;
                asm volatile("" ::: "memory");
                
                d1 = (double)f1 * d2 + sin(d3);
                v5 = (int)d1 ^ v6;
                
                // Memory operation with pointer arithmetic
                double* ptr = &mem_array[(v5 + inner) % 16];
                *ptr = d4 * d5 + (double)v7;
                asm volatile("" ::: "memory");
                
                // Call helper functions creating scheduling boundaries
                f4 = helper_float_op(f1, f2, f3);
                v8 = helper_int_op(v1, v5, inner);
                d6 = helper_mem_op(mem_array, v8, mid);
                
                // Conditional execution paths
                switch ((inner + outer) % 5) {
                    case 0:
                        // FP-intensive path
                        f5 = f6 * f7 + sinf(f8) * cosf(f9);
                        d7 = d8 * d9 + sqrt(d10);
                        v9 = (int)(f5 * 100.0f) ^ (int)(d7 * 50.0);
                        break;
                    case 1:
                        // Integer-intensive path
                        v10 = (v1 << 2) | (v2 >> 1);
                        v9 = v10 ^ v3 ^ v4 ^ v5;
                        v9 = (v9 * 1103515245 + 12345) & 0x7fffffff;
                        break;
                    case 2:
                        // Mixed operations
                        f10 = (float)v6 * 0.5f + f1;
                        d10 = (double)v7 * 0.25 + d2;
                        v9 = (int)(f10 * d10) % 1000;
                        break;
                    case 3:
                        // Memory-intensive path
                        for (int i = 0; i < 4; i++) {
                            mem_array[(i + inner) % 16] = 
                                mem_array[(i + mid) % 16] * 1.1;
                        }
                        v9 = (int)mem_array[inner % 16];
                        break;
                    default:
                        // Call-heavy path
                        f3 = helper_float_op(f2, f4, f5);
                        v9 = helper_int_op(v2, v4, v6);
                        d3 = helper_mem_op(mem_array, v9, outer);
                        v9 = (int)d3;
                        break;
                }
                
                // More dependency chains
                v2 = v3 ^ v9;
                f2 = f3 + (float)v2 * 0.1f;
                d2 = d3 * (double)f2;
                
                // Accumulate results
                accum_f += f1 + f2 + f3 + f4 + f5;
                accum_d += d1 + d2 + d3 + d4 + d5;
                counter1 += v1 + v2 + v3;
                counter2 += v4 + v5 + v6;
                counter3 += v7 + v8 + v9;
                
                asm volatile("" ::: "memory");
            }
        }
        
        // Additional conditional block outside inner loops
        if (outer % 3 == 0) {
            // Another mixed operation sequence
            v1 = v2 * v3 - v4;
            f1 = helper_float_op(f2, f3, f4);
            d1 = helper_mem_op(mem_array, v1, outer);
            v5 = helper_int_op(v1, (int)f1, (int)d1);
            
            // Complex floating point chain
            for (int i = 0; i < 3; i++) {
                f6 = f7 * f8 + sinf(f9 * (float)i);
                d6 = d7 * d8 + cos(d9 * (double)i);
                asm volatile("" ::: "memory");
            }
        } else if (outer % 3 == 1) {
            // Integer bit manipulation chain
            v6 = (v7 << (outer % 4)) | (v8 >> (outer % 4));
            v7 = v6 ^ ~v9;
            v8 = (v7 * 214013 + 2531011) & 0x7fffffff;
            v9 = v8 % 10007;
        }
    }
    
    // Final checksum calculation using all variables
    checksum = (uint64_t)v1 ^ (uint64_t)v2 << 8 ^ (uint64_t)v3 << 16;
    checksum ^= (uint64_t)v4 << 24 ^ (uint64_t)v5 << 32;
    checksum ^= (uint64_t)v6 << 40 ^ (uint64_t)v7 << 48;
    checksum ^= (uint64_t)v8 ^ (uint64_t)v9 << 8;
    checksum ^= *(uint64_t*)&f1 ^ *(uint64_t*)&f2;
    checksum ^= *(uint64_t*)&f3 ^ *(uint64_t*)&f4;
    checksum ^= *(uint64_t*)&d1 ^ *(uint64_t*)&d2;
    checksum ^= *(uint64_t*)&d3 ^ *(uint64_t*)&d4;
    checksum ^= (uint64_t)counter1 ^ (uint64_t)counter2 << 16;
    checksum ^= (uint64_t)counter3 << 32;
    checksum ^= *(uint64_t*)&accum_f ^ *(uint64_t*)&accum_d;
    
    return checksum;
}

int main() {
    volatile int iterations = 1000; // Volatile to prevent constant propagation
    
    printf("Starting complex scheduling stress test...\n");
    
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}

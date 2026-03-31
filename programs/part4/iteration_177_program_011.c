#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define NOINLINE __attribute__((noinline))
#define MEMORY_BARRIER asm volatile("" ::: "memory")

// Helper functions that won't be inlined
NOINLINE float helper_float_op(float a, float b, float c) {
    MEMORY_BARRIER;
    float r = sinf(a) * cosf(b) + tanf(c);
    MEMORY_BARRIER;
    return r * 0.5f;
}

NOINLINE int helper_int_op(int a, int b, int c) {
    MEMORY_BARRIER;
    int r = (a ^ b) | (b & c) | (c << 3);
    r = (r * 1103515245 + 12345) & 0x7fffffff;
    MEMORY_BARRIER;
    return r;
}

NOINLINE double helper_double_op(double a, double b, int c) {
    MEMORY_BARRIER;
    double r = sqrt(fabs(a)) * log(fabs(b) + 1.0);
    r += (c & 255) * 0.01;
    MEMORY_BARRIER;
    return r;
}

NOINLINE void helper_mem_op(int* arr, float* farr, int idx, float val) {
    MEMORY_BARRIER;
    arr[idx % 64] = (int)(val * 1000.0f) ^ idx;
    farr[idx % 32] = val * 0.9f;
    MEMORY_BARRIER;
}

// Main complex function with high register pressure
NOINLINE uint64_t complex_scheduling_function(volatile int outer_limit) {
    // Declare many local variables to create register pressure
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50;
    int i6 = 60, i7 = 70, i8 = 80, i9 = 90, i10 = 100;
    int i11 = 110, i12 = 120, i13 = 130, i14 = 140, i15 = 150;
    int i16 = 160, i17 = 170, i18 = 180, i19 = 190, i20 = 200;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    int* mem_ptr = (int*)malloc(64 * sizeof(int));
    float* fptr = (float*)malloc(32 * sizeof(float));
    
    // Initialize arrays
    for (int i = 0; i < 64; i++) mem_ptr[i] = i;
    for (int i = 0; i < 32; i++) fptr[i] = i * 0.5f;
    
    uint64_t checksum = 0;
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        MEMORY_BARRIER;
        
        // Nested loop 1 - integer operations with volatile dependency
        volatile int inner_limit1 = (outer % 10) + 5;
        for (int j = 0; j < inner_limit1; j++) {
            // Mixed integer operations creating dependencies
            i1 = i2 + i3;
            i2 = i1 ^ i4;
            i3 = i2 * i5;
            i4 = i3 | i6;
            i5 = i4 - i7;
            i6 = i5 & i8;
            
            MEMORY_BARRIER;
            
            // Call helper with integer operations
            i7 = helper_int_op(i1, i2, i3);
            
            // More integer operations
            i8 = i7 << (j % 4);
            i9 = i8 >> 2;
            i10 = i9 * 3;
            
            // Memory operations
            mem_ptr[(i10 + j) % 64] = i10;
            i11 = mem_ptr[(j + outer) % 64];
            
            MEMORY_BARRIER;
        }
        
        // Nested loop 2 - floating point operations
        int inner_limit2 = (outer % 8) + 3;
        for (int k = 0; k < inner_limit2; k++) {
            // Mixed float operations
            f1 = f2 * f3 + (float)k;
            f2 = f1 / (f4 + 1.0f);
            f3 = sinf(f2) * cosf(f5);
            f4 = f3 + f6 * 2.0f;
            
            MEMORY_BARRIER;
            
            // Call helper with float operations
            f5 = helper_float_op(f1, f2, f3);
            
            // More float operations
            f6 = f5 * 0.9f;
            f7 = f6 - f8;
            f8 = f7 * f9;
            
            // Convert to int and back
            i12 = (int)(f8 * 100.0f);
            f9 = (float)i12 * 0.01f;
            
            MEMORY_BARRIER;
        }
        
        // Conditional execution paths
        switch (outer % 7) {
            case 0:
                // Integer bit manipulation path
                i13 = (i13 << 1) | (i14 & 1);
                i14 = i13 ^ i15;
                i15 = helper_int_op(i13, i14, outer);
                for (int m = 0; m < 3; m++) {
                    i16 = (i16 * 1103515245 + 12345) & 0x7fffffff;
                    MEMORY_BARRIER;
                }
                break;
                
            case 1:
            case 2:
                // Floating point math path
                f10 = helper_float_op(f1, f3, f5);
                d1 = (double)f10 * 1.5;
                d2 = helper_double_op(d1, d2, outer);
                for (int m = 0; m < 2; m++) {
                    d3 = sqrt(d2 + (double)m);
                    MEMORY_BARRIER;
                }
                break;
                
            case 3:
            case 4:
                // Mixed operations path
                i17 = helper_int_op(i10, i11, i12);
                f10 = (float)i17 * 0.001f;
                helper_mem_op(mem_ptr, fptr, outer, f10);
                d4 = helper_double_op(d3, d4, i17);
                break;
                
            default:
                // Memory intensive path
                for (int m = 0; m < 4; m++) {
                    int idx = (outer + m) % 64;
                    mem_ptr[idx] = mem_ptr[(idx + 1) % 64] + m;
                    fptr[m % 32] = (float)mem_ptr[idx] * 0.1f;
                    MEMORY_BARRIER;
                }
                break;
        }
        
        // Update checksum with all variables
        checksum ^= (uint64_t)i1;
        checksum ^= (uint64_t)i2 << 8;
        checksum ^= (uint64_t)i3 << 16;
        checksum ^= (uint64_t)i4 << 24;
        checksum ^= (uint64_t)i5 << 32;
        checksum ^= (uint64_t)((int)f1) << 40;
        checksum ^= (uint64_t)((int)f2) << 48;
        checksum ^= (uint64_t)((int)d1) << 56;
        
        MEMORY_BARRIER;
    }
    
    // Final mixed operations
    d5 = helper_double_op(d1, d2, i20);
    i19 = helper_int_op(i18, i17, (int)d5);
    f10 = helper_float_op(f9, f8, (float)i19 * 0.01f);
    
    // Final memory operations
    for (int i = 0; i < 8; i++) {
        helper_mem_op(mem_ptr, fptr, i, f10 + (float)i);
        MEMORY_BARRIER;
    }
    
    // Final checksum accumulation
    for (int i = 0; i < 64; i++) {
        checksum ^= (uint64_t)mem_ptr[i] << (i % 16);
    }
    
    free(mem_ptr);
    free(fptr);
    
    return checksum;
}

int main() {
    // Volatile to prevent optimization
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling stress test...\n");
    
    // Run the complex function
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}

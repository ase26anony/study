#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define BARRIER() asm volatile("" ::: "memory")

// Non-inline helper functions to force scheduler state saves/restores
NOINLINE int helper1(int a, int b, float c, double d) {
    BARRIER();
    int r = (a * b) ^ (int)(c * 100.0f) ^ (int)(d * 1000.0);
    BARRIER();
    return r;
}

NOINLINE float helper2(float a, float b, int c, double d) {
    BARRIER();
    float r = (a * b) + (float)c + (float)d;
    BARRIER();
    return r;
}

NOINLINE double helper3(double a, int b, float c, int d) {
    BARRIER();
    double r = a / (b + 1) + (double)c * (d % 7);
    BARRIER();
    return r;
}

NOINLINE void memory_op(int* arr, float* farr, double* darr, int idx) {
    BARRIER();
    arr[idx] = (arr[idx] * 3) ^ (idx * 7);
    farr[idx] = farr[idx] * 1.5f + (float)idx;
    darr[idx] = darr[idx] / 2.0 - (double)idx;
    BARRIER();
}

// Main complex function with high register pressure
NOINLINE uint64_t complex_scheduling_function(volatile int outer_limit) {
    // Many local variables to create register pressure (30+)
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50;
    int i6 = 60, i7 = 70, i8 = 80, i9 = 90, i10 = 100;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    int* ptr1 = &i1;
    float* ptr2 = &f1;
    double* ptr3 = &d1;
    
    // Arrays for memory operations
    int arr[32];
    float farr[32];
    double darr[32];
    
    // Initialize arrays
    for (int i = 0; i < 32; i++) {
        arr[i] = i * 3;
        farr[i] = (float)i * 1.5f;
        darr[i] = (double)i * 2.5;
    }
    
    uint64_t checksum = 0;
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        BARRIER();
        
        // Nested loop level 1 - variable bound based on outer
        int inner1_limit = (outer % 8) + 3;
        for (int inner1 = 0; inner1 < inner1_limit; inner1++) {
            // Mixed operation dependency chain
            i1 = i2 * i3 + inner1;
            f1 = (float)i1 / 3.0f + f2;
            d1 = (double)f1 * 2.5 + d2;
            
            BARRIER();
            
            // Memory operations with dependencies
            arr[inner1] = arr[inner1] + i1;
            farr[inner1] = farr[inner1] * f1;
            darr[inner1] = darr[inner1] - d1;
            
            BARRIER();
            
            // Nested loop level 2 - depends on inner1
            for (int inner2 = 0; inner2 < (inner1 % 4) + 1; inner2++) {
                // Complex dependency chain across types
                i3 = (i4 ^ i5) + inner2;
                f3 = f4 * f5 + (float)i3;
                d3 = d4 / d5 - (double)f3;
                
                // Call helper functions creating scheduling boundaries
                i6 = helper1(i3, i4, f3, d3);
                f6 = helper2(f3, f4, i6, d3);
                d6 = helper3(d3, i6, f6, inner2);
                
                BARRIER();
                
                // Conditional execution paths
                switch (inner2 % 5) {
                    case 0:
                        // FP math path
                        f7 = f6 * 1.618f;
                        d7 = d6 / 3.14159;
                        i7 = (int)(f7 + d7);
                        break;
                    case 1:
                        // Integer bit manipulation path
                        i7 = (i6 << 3) ^ (i6 >> 2);
                        f7 = (float)(i7 & 0xFF);
                        d7 = (double)(i7 | 0x7F);
                        break;
                    case 2:
                        // Memory intensive path
                        memory_op(arr, farr, darr, inner2);
                        i7 = arr[inner2];
                        f7 = farr[inner2];
                        d7 = darr[inner2];
                        break;
                    case 3:
                        // Mixed operations
                        i7 = i6 * 7 - inner2;
                        f7 = (float)i7 * 0.25f;
                        d7 = (double)f7 * 1.5;
                        break;
                    default:
                        // All operations
                        i7 = helper1(i6, inner2, f6, d6);
                        f7 = helper2(f6, 2.0f, i7, d6);
                        d7 = helper3(d6, i7, f7, outer);
                }
                
                BARRIER();
                
                // More dependency chains
                i8 = i7 + i6 - i5;
                f8 = f7 * f6 / f5;
                d8 = d7 + d6 - d5;
                
                // Update checksum
                checksum ^= (uint64_t)i7;
                checksum ^= (uint64_t)(*(int*)&f7);
                checksum ^= (uint64_t)(*(long*)&d7);
            }
            
            BARRIER();
            
            // Additional operations in outer loop
            i9 = i8 * i7 % 97;
            f9 = f8 * 0.9f + (float)i9;
            d9 = d8 * 1.1 - (double)i9;
            
            // More helper calls
            i10 = helper1(i9, outer, f9, d9);
            f10 = helper2(f9, 3.14f, i10, d9);
            d10 = helper3(d9, i10, f10, inner1);
            
            // Update array with results
            arr[inner1 + 16] = i10;
            farr[inner1 + 16] = f10;
            darr[inner1 + 16] = d10;
        }
        
        BARRIER();
        
        // Final processing of arrays
        for (int i = 0; i < 16; i++) {
            i1 = arr[i] ^ arr[i + 16];
            f1 = farr[i] + farr[i + 16];
            d1 = darr[i] * darr[i + 16];
            
            checksum ^= (uint64_t)i1;
            checksum ^= (uint64_t)(*(int*)&f1);
            checksum ^= (uint64_t)(*(long*)&d1);
        }
    }
    
    return checksum;
}

int main() {
    // Volatile to prevent constant propagation
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling stress test...\n");
    
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}

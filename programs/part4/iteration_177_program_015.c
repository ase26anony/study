#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Non-inline helper functions to force scheduler state saves/restores
__attribute__((noinline)) 
int helper1(int a, int b, int c) {
    volatile int barrier = 0;
    int result = (a * b) + (c << 3);
    asm volatile("" : "+r"(result) : : "memory");
    return result ^ barrier;
}

__attribute__((noinline))
float helper2(float x, float y, int scale) {
    volatile float barrier = 0.0f;
    float result = (x * y) / (scale + 1);
    asm volatile("" : "+f"(result) : : "memory");
    return result + barrier;
}

__attribute__((noinline))
double helper3(double a, double b, int* ptr) {
    volatile double barrier = 0.0;
    double result = a * b + *ptr;
    asm volatile("" : "+f"(result) : : "memory");
    return result + barrier;
}

__attribute__((noinline))
void memory_op(int* dest, const int* src, int count) {
    volatile int barrier = 0;
    for (int i = 0; i < count; i++) {
        dest[i] = src[i] + barrier;
        asm volatile("" : : : "memory");
    }
}

// Main complex function with high register pressure
void complex_scheduling_test(void) {
    // Declare many variables to create high register pressure
    volatile int outer_limit = 1000;  // Prevent constant propagation
    
    // Integer variables (20+)
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    // Floating point variables
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    // Double precision variables
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    // Array for memory operations
    int array1[64], array2[64];
    for (int i = 0; i < 64; i++) {
        array1[i] = i;
        array2[i] = i * 2;
    }
    
    // Volatile variables to prevent optimization
    volatile int vol_counter = 0;
    volatile float vol_float = 0.0f;
    volatile double vol_double = 0.0;
    
    // Final checksum
    volatile uint64_t checksum = 0;
    
    // Outer loop with volatile limit
    for (int outer = 0; outer < outer_limit; outer++) {
        // Mix operations to create complex dependency chains
        v1 = v2 + v3;
        f1 = f2 * f3;
        d1 = d2 - d3;
        
        // Memory barrier to force scheduler to handle dependencies
        asm volatile("" : : : "memory");
        
        // Nested loops with variable bounds
        volatile int inner_limit = (outer % 16) + 1;
        
        for (int mid = 0; mid < inner_limit; mid++) {
            // More variable mixing
            v4 = v5 * v6;
            f4 = f5 / f6;
            d4 = d5 * d1;
            
            // Innermost loop with data-dependent trip count
            int innermost_limit = (v4 % 8) + 1;
            for (int inner = 0; inner < innermost_limit; inner++) {
                // Complex operation chain mixing types
                v7 = helper1(v8, v9, v10);
                f7 = helper2(f8, f9, v7);
                
                // Memory operation
                memory_op(array1, array2, 8);
                
                // More mixed operations
                v11 = v12 ^ v13;
                f10 = f7 * f3;
                d5 = helper3(d2, d3, &v14);
                
                // Barrier to prevent reordering
                asm volatile("" : : : "memory");
                
                // Conditional execution paths
                switch (inner % 4) {
                    case 0:
                        // FP math path
                        f1 = f2 * f3 + f4;
                        f5 = f6 / f7 - f8;
                        v15 = (int)(f1 * 100);
                        break;
                    case 1:
                        // Integer bit manipulation path
                        v16 = (v17 << 3) | (v18 >> 2);
                        v19 = v20 ^ ~v16;
                        v15 = v19 & 0xFFFF;
                        break;
                    case 2:
                        // Memory intensive path
                        for (int j = 0; j < 4; j++) {
                            array1[j] = array2[j + 4] + v15;
                            asm volatile("" : : : "memory");
                        }
                        v15 = array1[0] + array1[3];
                        break;
                    case 3:
                        // Mixed type path
                        v15 = (int)(d1 * d2);
                        f1 = (float)v15 / 100.0f;
                        d3 = (double)f1 * 1.5;
                        v15 = (int)d3;
                        break;
                }
                
                // Update volatile variables
                vol_counter++;
                vol_float += f1;
                vol_double += d1;
                
                // More dependency chains
                v20 = v15 + v1;
                v1 = v20 - v2;
                v2 = v1 * v3;
                v3 = v2 / (v4 + 1);
                
                f2 = f1 + f3;
                f3 = f2 * f4;
                f4 = f3 - f5;
                
                d2 = d1 + d3;
                d3 = d2 * d4;
                d4 = d3 / 2.0;
            }
            
            // Call helper with current state
            v8 = helper1(v9, v10, v11);
            f8 = helper2(f9, f10, v8);
            
            // Another barrier
            asm volatile("" : : : "memory");
        }
        
        // Update checksum with all variables
        checksum ^= (uint64_t)v1;
        checksum ^= (uint64_t)v2 << 8;
        checksum ^= (uint64_t)v3 << 16;
        checksum ^= (uint64_t)v4 << 24;
        checksum ^= (uint64_t)v5 << 32;
        checksum ^= (uint64_t)(f1 * 1000);
        checksum ^= (uint64_t)(d1 * 1000) << 16;
        
        // Prevent loop unrolling
        asm volatile("" : : : "memory");
    }
    
    // Final accumulation
    checksum += vol_counter;
    checksum += (uint64_t)(vol_float * 100);
    checksum += (uint64_t)(vol_double * 100);
    
    // Use all variables one more time
    v6 = v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    f6 = f7 + f8 + f9 + f10;
    d5 = d1 + d2 + d3 + d4;
    
    checksum ^= v6;
    checksum ^= (uint64_t)(f6 * 100);
    checksum ^= (uint64_t)(d5 * 100) << 32;
    
    // Print to prevent dead code elimination
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
}

int main(void) {
    complex_scheduling_test();
    return 0;
}

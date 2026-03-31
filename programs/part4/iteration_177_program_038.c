#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define NOINLINE __attribute__((noinline))

// Helper functions to prevent inlining and create scheduling boundaries
NOINLINE int helper1(int a, float b, volatile int* mem) {
    asm volatile("" : : "r"(a), "r"(b), "r"(mem) : "memory");
    int result = a + (int)(b * 2.0f);
    *mem = result;
    return result ^ (*mem);
}

NOINLINE float helper2(double d, int* arr, int idx) {
    asm volatile("" : : "r"(d), "r"(arr), "r"(idx) : "memory");
    float f = (float)d + (float)arr[idx % 16];
    arr[idx % 16] = (int)(f * 100.0f);
    return f;
}

NOINLINE void helper3(volatile int* counter, float* farr, int size) {
    asm volatile("" : : "r"(counter), "r"(farr), "r"(size) : "memory");
    for (int i = 0; i < size % 8; i++) {
        farr[i] = farr[i] * 1.1f + (float)(*counter);
        (*counter)++;
    }
}

int main(void) {
    // High register pressure: many live variables of different types
    volatile int outer_limit = 1000;  // Prevent constant propagation
    volatile int checksum = 0;
    
    // Many local variables to pressure register allocator
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15, v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    int v21 = 21, v22 = 22, v23 = 23, v24 = 24, v25 = 25, v26 = 26, v27 = 27, v28 = 28, v29 = 29, v30 = 30;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    int* mem_array = (int*)malloc(64 * sizeof(int));
    float* float_array = (float*)malloc(32 * sizeof(float));
    
    // Initialize arrays
    for (int i = 0; i < 64; i++) mem_array[i] = i;
    for (int i = 0; i < 32; i++) float_array[i] = (float)i * 0.5f;
    
    volatile int loop_counter = 0;
    
    // Outer loop with volatile limit
    for (int outer = 0; outer < outer_limit; outer++) {
        // Mixed type dependency chain
        v1 = v2 + v3;
        f1 = (float)v1 * f2;
        d1 = (double)f1 + d2;
        v4 = (int)d1 ^ v5;
        
        // Memory barrier
        asm volatile("" ::: "memory");
        
        // Nested loops with variable bounds
        volatile int inner_limit = (outer % 10) + 5;
        for (int mid = 0; mid < inner_limit; mid++) {
            // Inner loop with data-dependent trip count
            int inner_trip = (v4 + mid) % 8 + 2;
            for (int inner = 0; inner < inner_trip; inner++) {
                // Complex mixed operations
                v6 = v7 * v8 - v9;
                f3 = f4 * f5 + (float)v6;
                mem_array[(v6 + inner) % 64] = (int)(f3 * 100.0f);
                v10 = mem_array[(v7 + inner) % 64] ^ v8;
                f6 = sqrtf(fabsf(f3)) + (float)v10;
                d3 = (double)f6 * d4;
                v11 = (int)d3 | v12;
                
                // Another memory barrier
                asm volatile("" ::: "memory");
                
                // Conditional execution paths
                switch ((v11 + inner) % 4) {
                    case 0:
                        // FP math path
                        f7 = f8 * f9 - f10;
                        d5 = sin(d1) * cos(d2);
                        v13 = (int)(f7 * d5);
                        break;
                    case 1:
                        // Integer bit manipulation path
                        v14 = (v15 << 3) | (v16 >> 2);
                        v15 = v14 ^ ~v17;
                        v16 = (v15 * 1103515245 + 12345) & 0x7fffffff;
                        break;
                    case 2:
                        // Memory intensive path
                        for (int k = 0; k < 4; k++) {
                            int idx = (v18 + k) % 64;
                            mem_array[idx] = mem_array[idx] * 3 + k;
                            float_array[k] = (float)mem_array[idx] * 0.01f;
                        }
                        break;
                    case 3:
                        // Mixed type conversion path
                        f8 = (float)v19 + (float)v20;
                        v21 = (int)(f8 * 100.0f);
                        d2 = (double)v21 / 7.0;
                        v22 = (int)d2 ^ v23;
                        break;
                }
                
                // Call helper functions with cross-type dependencies
                v24 = helper1(v11, f3, &mem_array[(outer + inner) % 64]);
                f9 = helper2(d3, mem_array, v24 % 64);
                helper3(&loop_counter, float_array, v24);
                
                // Update checksum
                checksum ^= v1 ^ v4 ^ v6 ^ v10 ^ v11 ^ v13 ^ v14 ^ v15 ^ v16 ^ v21 ^ v22 ^ v24;
                checksum ^= (int)f1 ^ (int)f3 ^ (int)f6 ^ (int)f7 ^ (int)f8 ^ (int)f9;
                checksum ^= (int)d1 ^ (int)d2 ^ (int)d3 ^ (int)d4 ^ (int)d5;
            }
            
            // More mixed operations between loop levels
            v25 = v26 + v27 - v28;
            f10 = (float)v25 * 0.5f + f1;
            v26 = (int)(f10 * 100.0f) ^ v29;
            d4 = (double)v26 / 3.14159;
            
            // Memory barrier
            asm volatile("" ::: "memory");
        }
        
        // Update variables to create loop-carried dependencies
        v2 = v3 + 1;
        v3 = v4 ^ v5;
        v5 = v6 * 2 - v7;
        v7 = v8 | v9;
        v9 = v10 + v11;
        v12 = v13 - v14;
        v17 = v18 * v19;
        v18 = v20 ^ v21;
        v20 = v22 + v23;
        v23 = v24 * 3;
        v27 = v28 + v29;
        v28 = v30 ^ checksum;
        
        f2 = f3 * 1.1f;
        f4 = f5 + f6;
        f5 = f7 * 0.9f;
        
        d1 = d2 * 1.01;
        d2 = d3 + d4;
    }
    
    // Final computation to use all variables
    int final_result = 0;
    final_result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    final_result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    final_result += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    final_result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    final_result += (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10;
    final_result += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    
    checksum ^= final_result;
    
    // Print to prevent dead code elimination
    printf("Final checksum: %d\n", checksum);
    printf("Loop iterations: %d\n", loop_counter);
    
    free(mem_array);
    free(float_array);
    
    return 0;
}

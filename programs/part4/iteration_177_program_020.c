#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define BARRIER() asm volatile("" ::: "memory")

// Non-inline helper functions to force scheduler state saves/restores
NOINLINE int helper1(int a, int b, int c) {
    BARRIER();
    return (a ^ b) * c;
}

NOINLINE float helper2(float x, float y, int scale) {
    BARRIER();
    return (x + y) * scale;
}

NOINLINE double helper3(double a, double b, double c) {
    BARRIER();
    return (a * b) / c;
}

NOINLINE void memory_op(int* ptr, int idx, int val) {
    BARRIER();
    ptr[idx] = val;
    BARRIER();
}

int main() {
    // High register pressure: many live variables of different types
    volatile int outer_limit = 1000;  // Prevent constant propagation
    volatile int checksum = 0;
    
    // Integer variables
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    // Floating point variables
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    // Double precision variables
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    // Pointer/array variables
    int array[256];
    volatile int* volatile_ptr = array;  // Volatile pointer to prevent optimizations
    
    // Initialize array
    for (int i = 0; i < 256; i++) {
        array[i] = i;
    }
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        BARRIER();
        
        // Nested loop level 1 - variable bound based on outer
        int inner1_limit = (outer % 10) + 5;  // Variable trip count
        for (int i1 = 0; i1 < inner1_limit; i1++) {
            // Mixed operation dependency chain
            v1 = v2 + v3;
            BARRIER();
            f1 = (float)v1 * f2;
            BARRIER();
            memory_op(array, v1 % 256, (int)f1);
            BARRIER();
            v2 = volatile_ptr[v1 % 256] + v4;
            
            // Nested loop level 2 - depends on i1
            for (int i2 = 0; i2 < (i1 % 3) + 2; i2++) {
                // Complex conditional execution paths
                switch (i2 % 4) {
                    case 0:
                        // FP math path
                        f3 = helper2(f1, f2, v1);
                        d1 = helper3(d1, d2, d3);
                        v3 = (int)(f3 * d1);
                        break;
                    case 1:
                        // Integer bit manipulation path
                        v4 = helper1(v1, v2, v3);
                        v5 = (v4 << 3) | (v4 >> 5);
                        v6 = v5 ^ v4;
                        break;
                    case 2:
                        // Memory intensive path
                        v7 = array[i2 * 16];
                        v8 = array[i2 * 16 + 1];
                        memory_op(array, i2 * 16, v7 + v8);
                        v9 = volatile_ptr[i2 * 16];
                        break;
                    case 3:
                        // Mixed type computation
                        f4 = helper2(f3, f5, v10);
                        v11 = helper1(v7, v8, v9);
                        d2 = (double)v11 / f4;
                        v12 = (int)(d2 * 100.0);
                        break;
                }
                BARRIER();
                
                // More dependency chains
                f5 = f3 + f4;
                v13 = (int)f5;
                v14 = helper1(v13, v12, v11);
                f6 = helper2(f5, f4, v14);
                
                // Another nested loop level 3
                for (volatile int i3 = 0; i3 < 2; i3++) {
                    d3 = d1 * d2;
                    v15 = (int)d3;
                    v16 = array[v15 % 256];
                    v17 = helper1(v15, v16, v14);
                    f7 = (float)v17 * 0.5f;
                    BARRIER();
                }
            }
            
            // Update more variables to keep them live
            v18 = v17 + v16;
            v19 = v18 * v15;
            v20 = v19 ^ v14;
            
            f8 = f7 * 2.0f;
            f9 = helper2(f8, f6, v20);
            f10 = f9 / f5;
            
            d4 = (double)f10 * d3;
            d5 = helper3(d4, d2, d1);
        }
        
        // Accumulate to checksum with volatile write
        checksum ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
        checksum ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
        checksum ^= *(int*)&f1 ^ *(int*)&f2 ^ *(int*)&f3 ^ *(int*)&f4 ^ *(int*)&f5;
        checksum ^= *(int*)&f6 ^ *(int*)&f7 ^ *(int*)&f8 ^ *(int*)&f9 ^ *(int*)&f10;
        checksum ^= (int)d1 ^ (int)d2 ^ (int)d3 ^ (int)d4 ^ (int)d5;
        
        // Force memory barrier
        BARRIER();
    }
    
    // Final computation to use all variables
    int final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
        (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    
    checksum ^= final_result;
    
    // Print to prevent dead code elimination
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}

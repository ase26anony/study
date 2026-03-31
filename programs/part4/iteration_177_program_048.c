#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define BARRIER() asm volatile("" ::: "memory")

// Helper functions that won't be inlined
NOINLINE float fp_helper1(float a, float b, float c) {
    BARRIER();
    float r = (a * b) + (c / 1.5f);
    BARRIER();
    return r * 0.75f;
}

NOINLINE int int_helper1(int x, int y, int z) {
    BARRIER();
    int r = (x ^ y) | (z & 0x7F);
    BARRIER();
    return r + (y >> 3);
}

NOINLINE double mem_helper(double* arr, int idx1, int idx2) {
    BARRIER();
    double temp = arr[idx1] * 0.333 + arr[idx2] * 0.667;
    arr[idx1] = temp;
    BARRIER();
    return temp;
}

NOINLINE void mixed_helper(int* iarr, float* farr, volatile int* v) {
    BARRIER();
    *v = (*v + 1) & 0xFF;
    farr[0] = (float)iarr[0] * 1.1f;
    iarr[1] = (int)farr[1] ^ 0xABCD;
    BARRIER();
}

int main(void) {
    // Many local variables to create register pressure
    volatile int outer_limit = 1000;
    volatile int inner_limit_base = 50;
    volatile int checksum = 0;
    
    // Scalar variables of various types
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15, v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    int v21 = 21, v22 = 22, v23 = 23, v24 = 24, v25 = 25, v26 = 26, v27 = 27, v28 = 28, v29 = 29, v30 = 30;
    
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    // Arrays for memory operations
    int iarr[64];
    float farr[64];
    double darr[64];
    
    // Initialize arrays
    for (int i = 0; i < 64; i++) {
        iarr[i] = i * 3;
        farr[i] = (float)i * 1.5f;
        darr[i] = (double)i * 2.5;
    }
    
    // Outer loop with volatile limit
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        BARRIER();
        
        // Nested loops with variable bounds
        volatile int inner_limit = inner_limit_base + (outer & 0xF);
        
        for (int mid = 0; mid < 3; mid++) {
            for (volatile int inner = 0; inner < inner_limit; inner++) {
                // Complex dependency chains mixing types
                
                // Integer to float chain
                f1 = (float)v1 * 0.5f + f2;
                v1 = (int)f1 ^ v2;
                f2 = fp_helper1(f1, f3, f4);
                v2 = v1 * v3 + inner;
                
                // Float to memory chain
                darr[inner & 63] = (double)f2 * d1;
                f3 = (float)darr[(inner + 1) & 63] * 2.0f;
                v3 = (int)f3 | v4;
                
                // Memory to integer chain
                iarr[inner & 63] = v3 * v5 + mid;
                v4 = iarr[(inner + 2) & 63] ^ v6;
                f4 = (float)v4 * 0.25f;
                
                // Call helper with side effects
                d2 = mem_helper(darr, inner & 63, (inner + 3) & 63);
                
                // Conditional execution paths
                switch (inner & 7) {
                    case 0:
                        // FP math path
                        f5 = f1 * f2 - f3 / f4;
                        d3 = (double)f5 * 1.618;
                        v5 = (int)d3;
                        break;
                    case 1:
                        // Integer bit manipulation path
                        v6 = (v5 << 3) | (v6 >> 2);
                        v7 = v6 ^ 0xDEADBEEF;
                        v8 = (v7 * 1103515245 + 12345) & 0x7FFFFFFF;
                        break;
                    case 2:
                        // Mixed type path
                        f6 = (float)v8 * 0.333f;
                        v9 = int_helper1(v8, v9, v10);
                        d4 = (double)f6 * 2.71828;
                        break;
                    case 3:
                        // Memory intensive path
                        for (int j = 0; j < 4; j++) {
                            iarr[(inner + j) & 63] += v9;
                            farr[(inner + j) & 63] *= 1.01f;
                        }
                        break;
                    case 4:
                        // Another mixed path
                        v10 = v9 * v11 + v12;
                        f7 = fp_helper1(f5, f6, f7);
                        v11 = (int)f7 & v13;
                        break;
                    default:
                        // Default computation
                        v12 = v11 * 13 - v10;
                        f8 = (float)v12 * 0.123f;
                        v13 = int_helper1(v12, v13, v14);
                        break;
                }
                
                // More dependency chains
                v14 = v13 * v15 + outer;
                f9 = (float)v14 * 0.456f;
                v15 = (int)f9 ^ v16;
                
                d5 = d2 * d3 - d4;
                f10 = (float)d5 * 0.789f;
                v16 = (int)f10 | v17;
                
                // Call helper with multiple live variables
                mixed_helper(&v17, &f8, &checksum);
                
                // Continue chains
                v18 = v17 + v18 * 3;
                v19 = v18 ^ v19;
                v20 = v19 * v20 + inner;
                
                // More floating point
                f1 = f9 * 0.5f + f10 * 0.5f;
                f2 = fp_helper1(f1, f8, f3);
                
                v21 = v20 * 7 - v21;
                v22 = int_helper1(v21, v22, v23);
                
                // Memory store with computed index
                int idx = (v22 + inner) & 63;
                darr[idx] = (double)v22 * 0.001;
                iarr[idx] = v22;
                
                v23 = v22 * v23 + v24;
                v24 = v23 ^ 0x12345678;
                
                // Final chain segment
                v25 = v24 * v25 + mid;
                v26 = v25 | v26;
                v27 = v26 * 11 - v27;
                v28 = int_helper1(v27, v28, v29);
                v29 = v28 + v29 * 2;
                v30 = v29 ^ v30;
                
                BARRIER();
            }
            
            // Update checksum with multiple variables
            checksum ^= v1 ^ v5 ^ v10 ^ v15 ^ v20 ^ v25 ^ v30;
            checksum += (int)f1 + (int)f5 + (int)f10;
            checksum ^= (int)d1 ^ (int)d5;
        }
        
        // Update volatile variables to prevent optimization
        inner_limit_base = (inner_limit_base + 1) & 0x3F;
        BARRIER();
    }
    
    // Final checksum calculation using all variables
    int final_checksum = checksum;
    final_checksum ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    final_checksum ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    final_checksum ^= v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    final_checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    final_checksum += (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10;
    final_checksum ^= (int)d1 ^ (int)d2 ^ (int)d3 ^ (int)d4 ^ (int)d5;
    
    // Use array elements to prevent dead code elimination
    for (int i = 0; i < 64; i += 8) {
        final_checksum ^= iarr[i];
        final_checksum += (int)farr[i];
        final_checksum ^= (int)darr[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    return 0;
}

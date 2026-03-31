#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 128
#define DIM 16

typedef struct {
    int data[SIZE];
    double values[DIM][DIM];
    char* next;
} DataStruct;

// Force register usage with noinline attribute
__attribute__((noinline)) 
int helper_func(int a, int b, int c, int d, int e, float f, double g) {
    volatile int result;
    // Force register-to-register moves with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %1, %2" : "+r"(result) : "r"(c), "r"(d));
    return result + (int)(f * g);
}

int main() {
    // Initialize complex data structures
    DataStruct ds;
    int matrix1[DIM][DIM];
    int matrix2[DIM][DIM];
    double dmatrix[DIM][DIM];
    volatile int sink; // For forcing output reloads
    
    // Initialize data
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            matrix1[i][j] = i * DIM + j;
            matrix2[i][j] = (i + j) % 8;
            dmatrix[i][j] = (i * 0.5) + (j * 0.25);
            ds.values[i][j] = i * 1.5 + j * 0.75;
        }
    }
    
    for (int i = 0; i < SIZE; i++) {
        ds.data[i] = i * 3;
    }
    
    int result = 0;
    
    #pragma omp target map(to: matrix1, matrix2, dmatrix, ds) map(from: result)
    {
        // Create massive register pressure with many local variables
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        float f1, f2, f3, f4, f5, f6, f7, f8;
        double d1, d2, d3, d4, d5, d6;
        char c1, c2, c3, c4, c5;
        volatile int vsink1, vsink2, vsink3;
        
        // Initialize from mapped arrays with complex addressing
        v1 = matrix1[0][0];
        v2 = matrix2[0][0];
        v3 = ds.data[(v1 * 3 + v2) % SIZE];
        v4 = matrix1[1][1] + matrix2[1][1] * 2;
        
        // Complex pointer arithmetic for address reloads
        int* ptr1 = &matrix1[0][0];
        int* ptr2 = &matrix2[0][0];
        double* dptr = &dmatrix[0][0];
        
        // Nested loops creating live range interference
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++) {
                // Complex array indexing forcing address computation
                int idx1 = (i * DIM + j * 2) % DIM;
                int idx2 = (j * DIM + i) % DIM;
                
                // Chain computations keeping many variables live
                v5 = matrix1[idx1][idx2] + v1;
                v6 = matrix2[idx2][idx1] * v2;
                v7 = v3 + v4 - v5;
                v8 = v6 * 2 + v7 / 3;
                
                // Mixed type operations forcing mode conversions
                f1 = (float)v5 * 0.5f;
                f2 = (float)v6 * 1.5f;
                d1 = (double)v7 * 0.25;
                d2 = (double)v8 * 0.75;
                
                // Inline assembly with register constraints
                asm volatile ("add %0, %1, %2" : "=r"(v9) : "r"(v5), "r"(v6));
                asm volatile ("mul %0, %1, %2" : "+r"(v9) : "r"(v7), "r"(v8));
                
                // Force output reloads with volatile assignments
                vsink1 = v9;
                
                // More complex addressing with struct member access
                v10 = ds.data[(i * j) % SIZE] + matrix1[i][j];
                v11 = matrix2[j][i] * ds.data[(i + j) % SIZE];
                
                // Pointer arithmetic forcing address reloads
                v12 = *(ptr1 + i * DIM + j) + *(ptr2 + j * DIM + i);
                d3 = *(dptr + i * DIM + j) + ds.values[i][j];
                
                // Mixed precision operations
                f3 = f1 + f2 + (float)d1;
                f4 = (float)v10 * 0.3f + (float)v11 * 0.7f;
                
                // Force secondary reloads with mixed register class constraints
                #ifdef __aarch64__
                // Move between general and FP registers (may need secondary reload)
                asm volatile ("fmov %s0, %w1" : "=w"(f5) : "r"(v12));
                asm volatile ("fmov %w0, %s1" : "=r"(v13) : "w"(f3));
                #else
                // x86 version with xmm registers
                asm volatile ("movd %0, %1" : "=x"(f5) : "r"(v12));
                asm volatile ("movd %0, %1" : "=r"(v13) : "x"(f3));
                #endif
                
                // More variables to increase pressure
                v14 = v9 + v10 + v11 + v12 + v13;
                v15 = v14 * 2 - v9 / 2;
                v16 = v15 + matrix1[(i + 1) % DIM][(j + 1) % DIM];
                v17 = v16 * 3 + matrix2[(i + 2) % DIM][(j + 2) % DIM];
                
                // Char operations with different mode
                c1 = (char)(v14 & 0xFF);
                c2 = (char)(v15 & 0xFF);
                c3 = c1 + c2;
                c4 = c3 * 2;
                v18 = (int)c4 + v16;
                
                // Force output to memory with complex addressing
                int temp_idx = (i * 17 + j * 13) % DIM;
                vsink2 = matrix1[temp_idx][j] + v18;
                
                // Call helper function with many register arguments
                v19 = helper_func(v14, v15, v16, v17, v18, f3, d3);
                
                // Final computation chain
                v20 = v19 + v14 + v15 + v16 + v17 + v18;
                f6 = f3 + f4 + f5;
                d4 = d1 + d2 + d3;
                d5 = d4 * 0.5 + (double)f6;
                
                // Force another output reload
                vsink3 = (int)d5 + v20;
                
                // Update result
                result += v20 + (int)(f6 * 100.0f) + (int)d5;
            }
        }
        
        // Additional register pressure outside loops
        f7 = f1 + f2 + f3 + f4 + f5 + f6;
        d6 = d1 + d2 + d3 + d4 + d5;
        
        // More inline asm with memory constraints
        asm volatile ("ldr %0, [%1]" : "=r"(v1) : "r"(ptr1));
        asm volatile ("str %0, [%1]" : : "r"(v20), "r"(ptr2 + DIM * DIM - 1));
        
        // Complex expression with many live variables
        result += (int)(f7 * 10.0f) + (int)d6 + v1 + v2 + v3 + v4;
    }
    
    printf("Result: %d\n", result);
    return 0;
}

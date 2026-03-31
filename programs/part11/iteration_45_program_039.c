#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define DIM 16

typedef struct {
    int data[SIZE];
    double values[SIZE];
    char *ptr;
} DataStruct;

__attribute__((noinline))
int helper_func(int a, int b, float c, double d, char e, int f, float g, double h) {
    volatile int result;
    // Force register usage and potential reloads
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("fcvt %s0, %w1" : "=w"(c) : "r"(result)); // Force FP register usage
    return result + (int)c + (int)d + e + f;
}

int main() {
    // Initialize complex data structures
    int matrix1[DIM][DIM];
    double matrix2[DIM][DIM];
    DataStruct ds1, ds2;
    volatile int sink; // For forcing output reloads
    
    // Initialize data
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            matrix1[i][j] = i * DIM + j;
            matrix2[i][j] = (i * DIM + j) * 0.5;
        }
    }
    
    for (int i = 0; i < SIZE; i++) {
        ds1.data[i] = i;
        ds1.values[i] = i * 0.1;
        ds2.data[i] = SIZE - i;
        ds2.values[i] = (SIZE - i) * 0.1;
    }
    
    int result = 0;
    
    #pragma omp target map(to: matrix1, matrix2, ds1, ds2) map(from: result)
    {
        // Create massive register pressure with many local variables
        int v1 = matrix1[0][0];
        int v2 = matrix1[1][1];
        int v3 = matrix1[2][2];
        int v4 = matrix1[3][3];
        int v5 = matrix1[4][4];
        float f1 = matrix2[0][0];
        float f2 = matrix2[1][1];
        float f3 = matrix2[2][2];
        float f4 = matrix2[3][3];
        double d1 = matrix2[4][4];
        double d2 = matrix2[5][5];
        char c1 = v1 & 0xFF;
        char c2 = v2 & 0xFF;
        short s1 = v3 & 0xFFFF;
        short s2 = v4 & 0xFFFF;
        long l1 = v5 * 100L;
        long l2 = v1 * 200L;
        
        // Additional variables to increase pressure
        int v6, v7, v8, v9, v10;
        float f5, f6, f7;
        double d3, d4;
        
        // Complex nested loops with addressing that requires reloads
        for (int i = 0; i < DIM/2; i++) {
            for (int j = 0; j < DIM/2; j++) {
                // Complex array indexing forcing address computation
                int idx1 = (i * DIM + j * 3) % SIZE;
                int idx2 = (i * 7 + j * 11) % SIZE;
                
                // Chain computations keeping many variables live
                v6 = ds1.data[idx1] + ds2.data[idx2];
                v7 = matrix1[i][j] * matrix1[j][i];
                
                // Mixed type operations forcing mode conversions
                f5 = f1 + (float)v6 + (float)d1;
                d3 = d1 + (double)v7 + (double)f2;
                
                // Inline assembly with register constraints
                asm volatile ("add %0, %1, %2" : "=r"(v8) : "r"(v6), "r"(v7));
                asm volatile ("mul %0, %1, %2" : "=r"(v9) : "r"(v8), "r"(i));
                
                // Force FP register usage with constraints
                asm volatile ("fmul %s0, %s1, %s2" : "=w"(f6) : "w"(f5), "w"(f2));
                asm volatile ("fadd %d0, %d1, %d2" : "=w"(d4) : "w"(d3), "w"(d2));
                
                // Complex addressing with struct member access
                int *ptr1 = ds1.data;
                double *ptr2 = ds2.values;
                
                // Pointer arithmetic forcing address reloads
                v10 = ptr1[idx1 * 2] + ptr1[idx2 * 3];
                f7 = (float)ptr2[idx1] + (float)ptr2[idx2];
                
                // Force output reloads with volatile and computed addresses
                sink = v9 + v10;  // Volatile store
                
                // Assignment to array element with complex index
                matrix1[(i + j) % DIM][(i * j) % DIM] = v8 + v9;
                
                // Mixed type expression forcing conversions
                c1 = (c1 + c2 + (char)v8) & 0x7F;
                s1 = (s1 + s2 + (short)v9) & 0x7FFF;
                
                // Chain more computations
                l1 = l1 + v6 * v7 + v8 * v9;
                l2 = l2 + (long)(f5 * f6) + (long)(d3 * d4);
                
                // Call helper function with many register arguments
                v1 = helper_func(v6, v7, f5, d3, c1, v8, f6, d4);
                
                // More complex addressing
                int complex_idx = (i * 17 + j * 13 + v1) % SIZE;
                double complex_val = ds1.values[complex_idx] * ds2.values[complex_idx];
                
                // Force secondary reload scenarios
                asm volatile ("fmov %s0, %w1" : "=w"(f1) : "r"(v1)); // Move GPR to FP register
                asm volatile ("umov %w0, %v1.s[0]" : "=r"(v2) : "w"(f2)); // Move FP to GPR
                
                // Use builtins that may require specific registers
                v3 = __builtin_popcount(v1) + __builtin_clz(v2);
                
                // Keep all variables alive through the loop
                v4 = v1 + v2 + v3 + v4;
                f3 = f1 + f2 + f3 + f4;
                d1 = d1 + d2 + d3 + d4;
            }
        }
        
        // Final computation using all variables
        result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 +
                 (int)d1 + (int)d2 + (int)d3 + (int)d4 +
                 c1 + c2 + s1 + s2 + (int)(l1 & 0xFFFFFFFF) + (int)(l2 & 0xFFFFFFFF);
    }
    
    printf("Result: %d\n", result);
    return 0;
}

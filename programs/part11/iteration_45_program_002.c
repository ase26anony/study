#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define SIZE 128
#define DIM 16

typedef struct {
    int data[SIZE];
    double values[DIM][DIM];
    char *ptr;
} ComplexStruct;

__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = 0;
    // Force register usage with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %1, %2" : "+r"(result) : "r"(c), "r"(d));
    return result + e + f + g + h;
}

int main() {
    ComplexStruct cs;
    int matrix1[DIM][DIM];
    int matrix2[DIM][DIM];
    double dmatrix[DIM][DIM];
    char buffer[SIZE * 2];
    
    // Initialize data
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            matrix1[i][j] = i * DIM + j;
            matrix2[i][j] = (i + j) * 2;
            dmatrix[i][j] = i * 0.5 + j * 0.25;
            cs.values[i][j] = i * 1.1 + j * 0.9;
        }
    }
    
    for (int i = 0; i < SIZE; i++) {
        cs.data[i] = i * 3;
        buffer[i] = i % 128;
    }
    
    cs.ptr = buffer + SIZE;
    
    int result = 0;
    
    #pragma omp target map(to: matrix1, matrix2, dmatrix, cs) \
                      map(tofrom: result) \
                      map(tofrom: buffer[0:SIZE*2])
    {
        // Create massive register pressure with many local variables
        register int v1 = matrix1[0][0];
        register int v2 = matrix1[1][1];
        int v3 = matrix1[2][2];
        int v4 = matrix1[3][3];
        int v5 = matrix1[4][4];
        int v6 = matrix1[5][5];
        int v7 = matrix1[6][6];
        int v8 = matrix1[7][7];
        int v9 = matrix1[8][8];
        int v10 = matrix1[9][9];
        int v11 = matrix1[10][10];
        int v12 = matrix1[11][11];
        int v13 = matrix1[12][12];
        int v14 = matrix1[13][13];
        int v15 = matrix1[14][14];
        int v16 = matrix1[15][15];
        
        float f1 = dmatrix[0][0];
        float f2 = dmatrix[1][1];
        float f3 = dmatrix[2][2];
        float f4 = dmatrix[3][3];
        double d1 = cs.values[0][0];
        double d2 = cs.values[1][1];
        double d3 = cs.values[2][2];
        
        char c1 = buffer[0];
        char c2 = buffer[1];
        char c3 = buffer[2];
        char c4 = buffer[3];
        
        volatile int sink1, sink2, sink3;
        volatile double dsink;
        
        // Complex nested loops with addressing modes that require reloads
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++) {
                // Complex addressing with multiple computations
                int idx1 = (i * DIM + j) % SIZE;
                int idx2 = (i * 3 + j * 7) % SIZE;
                int idx3 = (i * 11 + j * 13) % DIM;
                
                // Force address reloads with pointer chains
                int *ptr1 = &cs.data[idx1];
                int *ptr2 = &matrix2[idx3][idx3];
                char *cptr = cs.ptr - idx2;
                
                // Mixed type computations creating mode conversions
                v1 = *ptr1 + v2;
                v2 = *ptr2 * v3;
                v3 = v4 - v5 + (int)c1;
                v4 = v6 / (v7 + 1);
                
                // Floating point operations forcing FP register pressure
                f1 = f2 * (float)v1 + f3;
                f2 = f4 / (float)(v2 + 1);
                d1 = d2 + (double)f1 * d3;
                
                // Inline assembly with register constraints
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "mul %0, %0, %3"
                    : "=r"(v5)
                    : "r"(v8), "r"(v9), "r"(v10)
                    : "cc"
                );
                
                // Secondary reload trigger: move between register classes
                int temp_for_fp;
                asm volatile (
                    "fmov %s0, %w1\n\t"
                    : "=w"(f3)
                    : "r"(v11)
                );
                
                asm volatile (
                    "fmov %w0, %s1\n\t"
                    : "=r"(v12)
                    : "w"(f4)
                );
                
                // Complex array access with computed indices
                double complex_val = cs.values[(i + j) % DIM][(i * j) % DIM] +
                                     dmatrix[idx3][(idx1 + idx2) % DIM];
                
                // Force output reloads with volatile stores
                sink1 = v1 + v2 + v3;
                sink2 = (int)(f1 * 100.0f);
                dsink = d1 + complex_val;
                
                // Assignment to computed pointer location (output reload)
                *cptr = (char)(v1 % 256);
                buffer[idx1] = (char)(v2 % 256);
                
                // Chain computations to keep variables live
                v6 = v5 + v12 + (int)c2;
                v7 = v6 * v13 - v14;
                v8 = v7 / (v15 + v16 + 1);
                v9 = v8 | v1 & v2;
                v10 = v9 ^ v3 ^ v4;
                v11 = v10 + (int)c3 * (int)c4;
                v13 = v11 - v12;
                v14 = v13 * 2;
                v15 = v14 + cs.data[idx2];
                v16 = v15 - matrix1[i][j];
                
                // Mixed type expression forcing mode conversion
                f4 = (float)v16 * 0.5f + (float)v15 * 0.25f;
                d3 = (double)f4 * 2.0 + d2;
                
                // Call helper function with many register arguments
                v1 = helper_func(v1, v2, v3, v4, v5, v6, v7, v8);
            }
        }
        
        // Final computation using all variables
        result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                 (int)d1 + (int)d2 + (int)d3 +
                 c1 + c2 + c3 + c4;
    }
    
    printf("Result: %d\n", result);
    return 0;
}

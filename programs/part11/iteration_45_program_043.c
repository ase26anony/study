#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define ITERS 100

typedef struct {
    int data[16];
    double values[8];
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
    // Initialize complex data structures
    int matrix1[SIZE][SIZE];
    int matrix2[SIZE][SIZE];
    double arr3d[32][32][32];
    ComplexStruct structs[64];
    volatile int sink = 0;  // For forcing output reloads
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * j;
            matrix2[i][j] = i + j;
        }
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 32; k++) {
                arr3d[i][j][k] = (i * 1.5) + (j * 2.3) + (k * 0.7);
            }
        }
    }
    
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 16; j++) {
            structs[i].data[j] = i * 100 + j;
        }
        for (int j = 0; j < 8; j++) {
            structs[i].values[j] = i * 0.5 + j * 1.1;
        }
    }
    
    int result = 0;
    
    #pragma omp target map(to: matrix1, matrix2, arr3d, structs) map(from: result)
    {
        // Create massive register pressure with many local variables
        register int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        register float f1, f2, f3, f4, f5, f6, f7, f8;
        register double d1, d2, d3, d4, d5, d6;
        register char c1, c2, c3, c4, c5;
        volatile int out1, out2, out3;  // For output reloads
        
        // Initialize from mapped arrays with complex addressing
        v1 = matrix1[0][0];
        v2 = matrix2[1][1];
        v3 = matrix1[2][2] + matrix2[3][3];
        
        // Complex 3D array access with modulo
        int idx1 = (v1 * 17 + v2) % 31;
        int idx2 = (v2 * 13 + v3) % 31;
        int idx3 = (v3 * 11 + v1) % 31;
        d1 = arr3d[idx1][idx2][idx3];
        
        // Struct member access with pointer chain simulation
        ComplexStruct *sptr = &structs[v1 % 64];
        v4 = sptr->data[(v2 * 3 + v3) % 16];
        d2 = sptr->values[(v1 + v2) % 8];
        
        // Start computation chain
        for (int iter = 0; iter < ITERS; iter++) {
            // Complex index calculations forcing address reloads
            int i = (iter * 7) % SIZE;
            int j = (iter * 13) % SIZE;
            int k = (iter * 19) % 32;
            
            // Multi-dimensional array access with complex expression
            v5 = matrix1[(i * 3 + j) % SIZE][(j * 5 + k) % SIZE];
            v6 = matrix2[(i * 7 + k) % SIZE][(j * 11 + i) % SIZE];
            
            // More complex addressing
            v7 = structs[(i + j) % 64].data[(i * j) % 16];
            v8 = structs[(j + k) % 64].data[(j * k) % 16];
            
            // Inline assembly with register constraints
            // Force input reloads with "r" constraints
            asm volatile ("add %0, %1, %2" : "=r"(v9) : "r"(v5), "r"(v6));
            asm volatile ("sub %0, %1, %2" : "=r"(v10) : "r"(v7), "r"(v8));
            
            // Mixed type operations forcing mode conversions
            f1 = (float)v9 * 1.5f;
            f2 = (float)v10 * 2.3f;
            
            // More inline asm with different constraints
            int temp;
            asm volatile ("mul %0, %1, %2" : "=r"(temp) : "r"(v9), "r"(v10));
            
            // Force output reload to memory with volatile
            out1 = temp;
            sink = out1;  // Use global volatile
            
            // Floating point operations
            f3 = f1 + f2;
            f4 = f1 - f2;
            f5 = f3 * f4;
            
            // Double precision operations
            d3 = d1 + d2;
            d4 = d1 - d2;
            d5 = d3 * d4;
            
            // Char operations mixed with ints
            c1 = (char)(v9 & 0xFF);
            c2 = (char)(v10 & 0xFF);
            c3 = c1 + c2;
            c4 = c1 - c2;
            c5 = c3 * c4;
            
            // More register pressure
            v1 = v1 + v5 + c5;
            v2 = v2 + v6 + c5;
            v3 = v3 + v7 + c5;
            v4 = v4 + v8 + c5;
            
            // Complex array store forcing output reload
            int store_idx = (i * 11 + j * 7 + k * 3) % 16;
            structs[i % 64].data[store_idx] = v1 + v2 + v3 + v4;
            
            // Volatile store forcing output reload
            out2 = f5;
            out3 = d5;
            sink = out2 + out3;
            
            // Function call with many register arguments
            int call_result = helper_func(v1, v2, v3, v4, v5, v6, v7, v8);
            
            // Use result in computation
            d6 = d5 + (double)call_result;
            
            // Secondary reload trigger: move between register classes
            // This might require secondary reloads on some architectures
            double d_temp;
            asm volatile ("/* potential reg class move */" : "=r"(v1) : "r"(call_result));
            
            // More mixed operations
            f6 = f5 + (float)d6;
            f7 = f6 * 2.0f;
            f8 = f7 / 3.0f;
            
            // Final complex computation
            result += v1 + v2 + v3 + v4 + (int)f8 + (int)d6 + c5;
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}

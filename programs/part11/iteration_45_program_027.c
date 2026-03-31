#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define NUM_VARS 25

typedef struct {
    int data[SIZE];
    double values[SIZE];
    char *ptr;
} ComplexStruct;

__attribute__((noinline))
int helper_function(int a, int b, int c, int d, int e, float f, double g) {
    volatile int result;
    // Force register usage and potential reloads
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %1, %2" : "+r"(result) : "r"(c), "r"(d));
    return result + (int)(f * g);
}

int main() {
    // Initialize complex data structures
    ComplexStruct cs1, cs2;
    int matrix1[SIZE][SIZE];
    int matrix2[SIZE][SIZE];
    double arr3d[SIZE][SIZE][2];
    
    // Initialize with some data
    for (int i = 0; i < SIZE; i++) {
        cs1.data[i] = i;
        cs2.data[i] = SIZE - i;
        cs1.values[i] = i * 1.5;
        cs2.values[i] = i * 2.5;
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * j;
            matrix2[i][j] = i + j;
            for (int k = 0; k < 2; k++) {
                arr3d[i][j][k] = (i * j * k) / 3.0;
            }
        }
    }
    
    cs1.ptr = (char*)matrix1;
    cs2.ptr = (char*)matrix2;
    
    int result = 0;
    
    #pragma omp target map(to: cs1, cs2, matrix1, matrix2, arr3d) map(from: result)
    {
        // Declare many local variables to create register pressure
        register int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        float f1, f2, f3, f4, f5, f6, f7, f8;
        double d1, d2, d3, d4, d5;
        char c1, c2, c3, c4;
        volatile int sink1, sink2, sink3;
        volatile double dsink;
        
        // Initialize from mapped data with complex addressing
        v1 = cs1.data[0];
        v2 = cs2.data[SIZE-1];
        f1 = (float)cs1.values[10];
        f2 = (float)cs2.values[20];
        d1 = arr3d[0][0][0];
        d2 = arr3d[1][1][1];
        
        // Complex nested loop with register pressure
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                // Complex array indexing forcing address computation
                int idx1 = (i * 37 + j * 13) % SIZE;
                int idx2 = (i * 19 + j * 29) % SIZE;
                int idx3 = (i * 23 + j * 31) % SIZE;
                
                // Chain computations creating long live ranges
                v3 = matrix1[idx1][idx2] + matrix2[idx2][idx3];
                v4 = cs1.data[idx1] * cs2.data[idx2];
                
                // Mixed type operations forcing mode conversions
                f3 = f1 + (float)v3 * 0.5f;
                f4 = f2 + (float)v4 * 0.25f;
                
                // Inline assembly with register constraints
                asm volatile ("add %0, %1, %2" : "=r"(v5) : "r"(v3), "r"(v4));
                asm volatile ("mul %0, %1, %2" : "+r"(v5) : "r"(i), "r"(j));
                
                // Force output reloads with volatile assignments
                sink1 = v5;
                
                // Complex pointer arithmetic
                char *ptr1 = cs1.ptr + idx1 * sizeof(int);
                char *ptr2 = cs2.ptr + idx2 * sizeof(int);
                
                // More mixed operations
                d3 = d1 + (double)v5 * 0.01;
                d4 = d2 + (double)(matrix1[idx3][idx1]) * 0.02;
                
                // Force secondary reloads with mixed register classes
                int temp_int;
                double temp_double;
                asm volatile ("fcvt %s0, %w1" : "=w"(temp_double) : "r"(v5)); // Integer to float
                asm volatile ("fcvtzs %w0, %s1" : "=r"(temp_int) : "w"(d3)); // Float to integer
                
                // More chaining
                v6 = v5 + temp_int;
                v7 = matrix2[idx1][idx2] - matrix1[idx2][idx3];
                
                // Complex struct member access
                v8 = cs1.data[(i + j) % SIZE] + cs2.data[(i * j) % SIZE];
                
                // 3D array access with complex index
                d5 = arr3d[idx1 % 16][idx2 % 16][(i + j) % 2] + 
                     arr3d[idx2 % 16][idx3 % 16][(i * j) % 2];
                
                // More inline assembly with memory constraints
                asm volatile ("ldr %0, [%1]" : "=r"(v9) : "r"(ptr1));
                asm volatile ("str %0, [%1]" : : "r"(v6), "r"(ptr2));
                
                // Force output to array with computed index
                int *out_ptr = &matrix1[(i + j) % 32][(i * j) % 32];
                *out_ptr = v6 + v7 + v8;
                
                // Volatile store forcing memory output
                dsink = d3 + d4 + d5;
                
                // More variables to increase pressure
                v10 = v6 * v7 / (v8 + 1);
                f5 = f3 * f4;
                f6 = (float)d3 * 2.0f;
                c1 = (char)(v9 & 0xFF);
                c2 = (char)(v10 & 0xFF);
                
                // Mixed type expression
                f7 = f5 + (float)c1 + (float)c2;
                
                // Call helper function with many register arguments
                v1 = helper_function(v3, v4, v5, v6, v7, f3, d3);
                
                // Chain results
                v2 = v1 + v9 + v10;
                sink2 = v2;
                
                // More complex addressing
                double *dptr = &arr3d[i % 8][j % 8][(i + j) % 2];
                *dptr = d3 * d4 - d5;
                
                // Final sink
                sink3 = (int)(f7 * 100.0f);
            }
        }
        
        // Compute final result
        result = v1 + v2 + sink1 + sink2 + sink3 + (int)dsink;
    }
    
    printf("Result: %d\n", result);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define NUM_VARS 20

typedef struct {
    int data[SIZE];
    double values[SIZE];
    char *ptr;
} ComplexStruct;

__attribute__((noinline))
int helper_function(int a, int b, float c, double d, char e, 
                    int f, int g, float h, double i, char j) {
    volatile int result;
    // Force register usage and potential reloads
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("fadd %s0, %s1, %s2" : "=w"(c) : "w"(c), "w"(h));
    return result + (int)c + (int)d + e + f + g + (int)i + j;
}

int main() {
    // Initialize complex data structures
    ComplexStruct cs;
    int matrix[SIZE][SIZE];
    double arr3d[SIZE][SIZE][SIZE];
    volatile int sink;
    
    // Initialize with some data
    for (int i = 0; i < SIZE; i++) {
        cs.data[i] = i;
        cs.values[i] = i * 1.5;
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
            for (int k = 0; k < SIZE; k++) {
                arr3d[i][j][k] = (i * j * k) / 3.14159;
            }
        }
    }
    
    cs.ptr = (char*)malloc(SIZE * sizeof(char));
    for (int i = 0; i < SIZE; i++) {
        cs.ptr[i] = i % 256;
    }
    
    int result = 0;
    
    #pragma omp target map(to: cs, matrix, arr3d) map(tofrom: result) \
                      map(tofrom: cs.ptr[0:SIZE])
    {
        // Declare many local variables to create register pressure
        register int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
        float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
        double d0, d1, d2, d3, d4;
        char c0, c1, c2, c3, c4;
        
        // Initialize from mapped data with complex addressing
        v0 = cs.data[0];
        v1 = matrix[0][0];
        d0 = cs.values[0];
        c0 = cs.ptr[0];
        
        // Complex nested loops with register pressure
        for (int i = 1; i < 16; i++) {
            for (int j = 1; j < 16; j++) {
                // Complex array indexing forcing address reloads
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 19) % SIZE;
                int idx3 = (i * 29 + j * 31) % SIZE;
                
                // Chain many computations to keep variables live
                v2 = matrix[idx1][idx2] + cs.data[idx3];
                v3 = v0 * v1 - v2;
                v4 = v2 / (v1 + 1) + v3;
                v5 = v3 ^ v4 | v2;
                v6 = v4 & v5 << 2;
                v7 = v5 >> 1 + v6;
                v8 = v6 * v7 - v4;
                v9 = v7 + v8 * v3;
                
                // Mixed type computations forcing mode conversions
                f0 = (float)v2 * 1.5f + (float)v3;
                f1 = f0 * 2.0f - (float)v4;
                f2 = f1 / 3.0f + (float)v5;
                f3 = f2 * f1 - f0;
                f4 = f3 + (float)v6 / f2;
                
                d1 = (double)f0 * 2.5 + d0;
                d2 = d1 * 3.14159 + (double)v7;
                d3 = d2 / 1.414 + (double)v8;
                d4 = d3 * d2 - d1;
                
                c1 = (char)(v2 % 256);
                c2 = c0 + c1;
                c3 = c2 * 2 - c1;
                c4 = c3 ^ c2;
                
                // Inline assembly with register constraints
                // Force specific register allocation conflicts
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %3, %4, %5"
                    : "=r"(v0), "=r"(v1), "+r"(v2)
                    : "r"(v3), "r"(v4), "r"(v5)
                    : "cc"
                );
                
                // Floating point inline assembly
                asm volatile (
                    "fadd %s0, %s1, %s2\n\t"
                    "fmul %s3, %s4, %s5"
                    : "=w"(f5), "=w"(f6)
                    : "w"(f0), "w"(f1), "w"(f2), "w"(f3)
                );
                
                // Mixed register class constraints
                int temp_int;
                float temp_float;
                asm volatile (
                    "fcvt %s0, %w1\n\t"    // Convert int to float
                    "fcvtzs %w2, %s3"      // Convert float to int
                    : "=w"(temp_float), "=r"(temp_int)
                    : "r"(v6), "w"(f4)
                );
                
                // Force output reloads with complex addressing
                // Assign to dereferenced pointer with computed address
                char *ptr = cs.ptr + ((i * 37 + j * 41) % SIZE);
                *ptr = (char)((v0 + v1 + v2) % 256);
                
                // Assign to array with complex index
                int arr_idx = (i * 43 + j * 47) % SIZE;
                cs.data[arr_idx] = v3 + v4 + v5;
                
                // Volatile store forcing memory output
                sink = v6 + v7 + v8;
                
                // Complex 3D array access
                int idx_i = (i * 53) % SIZE;
                int idx_j = (j * 59) % SIZE;
                int idx_k = ((i + j) * 61) % SIZE;
                d0 = arr3d[idx_i][idx_j][idx_k] + d4;
                
                // Struct member access with pointer chain
                double *val_ptr = cs.values + ((i * 67 + j * 71) % SIZE);
                *val_ptr = d1 * d2 - d3;
                
                // Call helper function with many register arguments
                // This forces calling convention handling and potential reloads
                int call_result = helper_function(
                    v0, v1, f0, d0, c0,
                    v2, v3, f1, d1, c1
                );
                
                // Chain result into computation
                v0 = v0 ^ call_result;
                v1 = v1 + call_result;
                
                // More inline assembly with memory constraints
                int mem_val;
                asm volatile (
                    "ldr %0, [%1]\n\t"
                    "add %0, %0, #1\n\t"
                    "str %0, [%1]"
                    : "=r"(mem_val), "+r"(ptr)
                    :
                    : "memory"
                );
            }
        }
        
        // Final computation using all variables
        result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                 (int)f5 + (int)f6 + (int)f7 + (int)f8 + (int)f9 +
                 (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4 +
                 c0 + c1 + c2 + c3 + c4;
    }
    
    printf("Result: %d\n", result);
    free(cs.ptr);
    
    return 0;
}

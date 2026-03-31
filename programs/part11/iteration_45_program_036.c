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
int helper_function(int a, int b, int c, int d, int e, 
                    float f, float g, double h, double i) {
    volatile int result;
    // Force register usage and potential reloads
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %1, %2" : "+r"(result) : "r"(c), "r"(d));
    return result + (int)(f * g) + (int)(h * i);
}

int main() {
    // Initialize complex data structures
    ComplexStruct cs1, cs2;
    int matrix1[SIZE][SIZE];
    int matrix2[SIZE][SIZE];
    double dmatrix[SIZE][SIZE];
    volatile int sink;
    
    // Initialize with some data
    for (int i = 0; i < SIZE; i++) {
        cs1.data[i] = i;
        cs2.data[i] = SIZE - i;
        cs1.values[i] = i * 1.5;
        cs2.values[i] = i * 2.5;
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * j;
            matrix2[i][j] = i + j;
            dmatrix[i][j] = i * j * 0.5;
        }
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: cs1, cs2, matrix1, matrix2, dmatrix) \
                      map(from: checksum)
    {
        // Declare many local variables to create register pressure
        int v1 = cs1.data[0], v2 = cs1.data[1], v3 = cs1.data[2];
        int v4 = cs1.data[3], v5 = cs1.data[4], v6 = cs1.data[5];
        int v7 = cs1.data[6], v8 = cs1.data[7], v9 = cs1.data[8];
        int v10 = cs1.data[9], v11 = cs1.data[10], v12 = cs1.data[11];
        
        float f1 = v1 * 0.1f, f2 = v2 * 0.2f, f3 = v3 * 0.3f;
        float f4 = v4 * 0.4f, f5 = v5 * 0.5f, f6 = v6 * 0.6f;
        
        double d1 = cs1.values[0], d2 = cs1.values[1], d3 = cs1.values[2];
        double d4 = cs1.values[3], d5 = cs1.values[4], d6 = cs1.values[5];
        
        char c1 = v1 & 0xFF, c2 = v2 & 0xFF, c3 = v3 & 0xFF;
        char c4 = v4 & 0xFF, c5 = v5 & 0xFF, c6 = v6 & 0xFF;
        
        // Additional variables for more pressure
        register int r1 = v7, r2 = v8, r3 = v9;
        register float rf1 = f3, rf2 = f4;
        register double rd1 = d3, rd2 = d4;
        
        // Complex nested loops with mixed operations
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                // Complex array indexing forcing address reloads
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 19 + j * 11) % SIZE;
                int idx3 = (i * 23 + j * 7) % SIZE;
                
                // Chain computations keeping many variables live
                v1 = matrix1[idx1][idx2] + v2;
                v2 = matrix2[idx2][idx3] * v3;
                v3 = v1 ^ v2;
                
                // Mixed type operations forcing mode conversions
                f1 = (float)v1 * 0.5f + f2;
                f2 = (float)v2 * 0.3f + f3;
                d1 = (double)v3 * 0.25 + d2;
                
                // Inline assembly with register constraints
                int temp1, temp2;
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "mul %3, %0, %4"
                    : "=r"(temp1), "+r"(v4), "+r"(v5)
                    : "r"(v6), "r"(v7)
                    : "cc"
                );
                
                // More assembly with different constraints
                double ftemp;
                asm volatile (
                    "fcvt %d0, %s1\n\t"
                    "fadd %d0, %d0, %d2"
                    : "=w"(ftemp)
                    : "w"(f1), "w"(d1)
                );
                
                // Force output reloads with volatile and pointer dereference
                sink = v1 + v2 + v3;
                
                // Complex struct member access with pointer chain
                int *ptr1 = &cs1.data[idx1];
                int *ptr2 = &cs2.data[idx2];
                *ptr1 = v4 + *ptr2;  // Forces address computation and store
                
                // Multi-dimensional array with complex index
                dmatrix[(i * 3 + j) % SIZE][(i + j * 2) % SIZE] = 
                    d1 + d2 + d3 + ftemp;
                
                // Mixed type expression
                v5 = (int)(f1 * 100.0f) + (int)(d1 * 50.0) + c1 + c2;
                
                // Use builtin that may require specific registers
                v6 = __builtin_popcount(v5) + v7;
                
                // More register pressure
                v7 = v8 ^ v9;
                v8 = v9 + v10;
                v9 = v10 * v11;
                v10 = v11 - v12;
                
                // Float/double operations
                f3 = f4 * f5;
                f4 = f5 + f6;
                d2 = d3 * d4;
                d3 = d4 / d5;
                
                // Char operations with sign extension
                c3 = (c1 + c2) & 0x7F;
                c4 = (c2 - c3) & 0x7F;
                
                // Call helper function with many register arguments
                v11 = helper_function(v1, v2, v3, v4, v5, f1, f2, d1, d2);
                
                // Chain more computations
                v12 = v11 + matrix1[idx3][idx1] + matrix2[idx1][idx3];
                
                // Update checksum
                checksum += v1 + v2 + v3 + v4 + v5 + v6 + 
                           (int)f1 + (int)f2 + (int)d1 + (int)d2 +
                           c1 + c2 + c3 + c4;
            }
        }
        
        // Final complex expression using all variables
        int final_result = 
            v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 +
            (int)(f1 + f2 + f3 + f4 + f5 + f6) +
            (int)(d1 + d2 + d3 + d4 + d5 + d6) +
            c1 + c2 + c3 + c4 + c5 + c6 +
            r1 + r2 + r3 + (int)rf1 + (int)rf2 + (int)rd1 + (int)rd2;
        
        sink = final_result;
        checksum += final_result;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

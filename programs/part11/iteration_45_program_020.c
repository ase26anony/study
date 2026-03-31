#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 128
#define DIM 16

typedef struct {
    int data[SIZE];
    double values[DIM][DIM];
    char *ptr;
} DataStruct;

/* Non-inline helper to force register usage for arguments */
__attribute__((noinline)) 
int compute_helper(int a, int b, int c, int d, int e, float f, double g) {
    volatile int result;
    // Force register pressure with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %0, %3" : "+r"(result) : "r"(c));
    return result + (int)(f * g) + d + e;
}

int main() {
    // Initialize complex data structures
    DataStruct ds;
    int matrix1[DIM][DIM], matrix2[DIM][DIM];
    double big_array[SIZE][SIZE];
    volatile int sink; // For forcing output reloads
    
    // Initialize with pattern
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            matrix1[i][j] = i * DIM + j;
            matrix2[i][j] = (i + j) % DIM;
            ds.values[i][j] = (i * 0.1) + (j * 0.01);
        }
    }
    
    for (int i = 0; i < SIZE; i++) {
        ds.data[i] = i * 3;
        for (int j = 0; j < SIZE; j++) {
            big_array[i][j] = (i + j) * 0.5;
        }
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix1, matrix2, ds, big_array) map(from: checksum)
    {
        // Declare many local variables to create register pressure
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
        float f1, f2, f3, f4, f5, f6, f7;
        double d1, d2, d3, d4, d5;
        char c1, c2, c3, c4;
        volatile int vsink; // Volatile inside target region
        
        // Initialize from mapped data with complex addressing
        v1 = matrix1[0][0];
        v2 = matrix2[1][1];
        v3 = ds.data[(v1 + v2) % SIZE];
        f1 = (float)ds.values[2][3];
        d1 = big_array[v1 % SIZE][v2 % SIZE];
        
        // Complex nested loop creating many live ranges
        for (int i = 0; i < DIM/2; i++) {
            for (int j = 0; j < DIM/2; j++) {
                // Complex array indexing forcing address reloads
                int idx1 = (i * DIM + j * 3) % DIM;
                int idx2 = (j * DIM + i * 7) % DIM;
                
                // Chain computations keeping many variables live
                v4 = matrix1[idx1][idx2] + matrix2[idx2][idx1];
                v5 = v4 * v3 - v2;
                v6 = v5 / (v1 + 1);
                
                // Mixed type operations forcing mode conversions
                f2 = f1 + (float)v4 * 0.5f;
                d2 = d1 + (double)v5 * 0.25;
                
                // Inline assembly with register constraints
                asm volatile ("add %0, %1, %2" : "=r"(v7) : "r"(v4), "r"(v5));
                asm volatile ("mul %0, %1, %2" : "=r"(v8) : "r"(v6), "r"(v7));
                
                // More variables to increase pressure
                v9 = v8 << 2;
                v10 = v9 ^ v7;
                f3 = f2 * 1.1f;
                d3 = d2 * 1.01;
                c1 = (char)(v10 & 0xFF);
                
                // Force output reload with volatile and complex addressing
                vsink = v10;
                ds.data[(i * j) % SIZE] = v9; // Assignment to array element
                
                // More arithmetic chains
                v11 = v10 + c1;
                v12 = v11 * matrix1[i][j];
                f4 = f3 + (float)ds.values[i][j];
                d4 = d3 - big_array[i][j][i % 8]; // Multi-dimensional
                
                // Secondary reload trigger: mixed register classes
                #ifdef __aarch64__
                // Force move between general and FP registers
                asm volatile ("fmov %s0, %w1" : "=w"(f5) : "r"(v12));
                asm volatile ("fmul %s0, %s0, %s1" : "=w"(f6) : "w"(f5), "w"(f4));
                #else
                // x86 version with xmm registers
                asm volatile ("movd %0, %1" : "=x"(f5) : "r"(v12));
                asm volatile ("mulss %0, %1" : "+x"(f5) : "x"(f4));
                f6 = f5;
                #endif
                
                // More live variables
                v13 = (int)f6;
                v14 = v13 + v12;
                v15 = v14 * 3;
                f7 = (float)v15 * 0.33f;
                d5 = (double)v15 + d4;
                c2 = (char)(v15 % 256);
                c3 = c1 + c2;
                c4 = c3 * 2;
                
                // Call helper function forcing register arguments
                int helper_result = compute_helper(v13, v14, v15, 
                                                  matrix1[i][j], matrix2[j][i],
                                                  f7, d5);
                
                // Final assignment with complex addressing
                int complex_idx = (i * DIM * 3 + j * 7) % SIZE;
                ds.data[complex_idx] = helper_result + c4;
                
                checksum += ds.data[complex_idx];
            }
        }
        
        // Additional computations outside loops keeping variables live
        int final1 = v15 + v14 + v13;
        float final2 = f7 + f6 + f5;
        double final3 = d5 + d4 + d3;
        
        // Force more output reloads
        vsink = final1;
        ds.data[0] = (int)final2;
        ds.data[1] = (int)final3;
        
        checksum += final1 + (int)final2 + (int)final3;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 128
#define NUM_VARS 20

struct DataNode {
    int data[SIZE];
    double values[SIZE];
    struct DataNode *next;
};

__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = 0;
    // Force register usage with inline assembly
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h)
        : "cc"
    );
    return result;
}

int main() {
    int matrix[SIZE][SIZE];
    double arr3d[SIZE][SIZE][SIZE];
    struct DataNode nodes[SIZE];
    volatile int sink;
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
        for (int j = 0; j < SIZE; j++) {
            for (int k = 0; k < SIZE; k++) {
                arr3d[i][j][k] = (i * 1.0) + (j * 0.5) + (k * 0.25);
            }
        }
        for (int j = 0; j < SIZE; j++) {
            nodes[i].data[j] = i ^ j;
            nodes[i].values[j] = (double)(i * j) / 100.0;
        }
        nodes[i].next = (i < SIZE - 1) ? &nodes[i + 1] : NULL;
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, arr3d, nodes) map(from: checksum)
    {
        // Declare many local variables to create register pressure
        register int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
        register int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
        float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
        double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
        char c0, c1, c2, c3, c4, c5, c6, c7, c8, c9;
        
        // Initialize from mapped arrays with complex addressing
        v0 = matrix[0][0];
        v1 = matrix[1][SIZE-1];
        v2 = matrix[SIZE/2][SIZE/3];
        
        // Complex pointer chain access
        struct DataNode *current = &nodes[0];
        v3 = current->data[0];
        v4 = current->next->data[1];
        v5 = current->next->next->data[2];
        
        // Mixed type computations
        f0 = (float)v0 * 1.5f;
        d0 = (double)v1 * 2.5;
        c0 = (char)(v2 & 0xFF);
        
        // Create long live ranges by chaining computations
        for (int i = 0; i < SIZE/4; i++) {
            for (int j = 0; j < SIZE/4; j++) {
                // Complex array indexing forcing address reloads
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 19 + j * 11) % SIZE;
                int idx3 = (i * 23 + j * 7) % SIZE;
                
                // Force output reloads with pointer dereference
                int *ptr = &matrix[idx1][idx2];
                *ptr = v0 + v1 + v2;  // out operand with computed address
                
                // More complex addressing with struct member
                double *dptr = &current->values[idx3];
                *dptr = d0 + arr3d[i][j][idx1] * 2.0;  // Another out operand
                
                // Inline assembly with register constraints
                int temp1, temp2, temp3;
                asm volatile (
                    "mul %0, %1, %2\n\t"
                    "add %0, %0, %3"
                    : "=r"(temp1)
                    : "r"(v3), "r"(v4), "r"(v5)
                    : "cc"
                );
                
                // Secondary reload trigger: mixed register classes
                float fval;
                int ival = temp1;
                // Force move between different register classes
                #ifdef __aarch64__
                asm volatile (
                    "fmov %s0, %w1"
                    : "=w"(fval)
                    : "r"(ival)
                );
                #elif defined(__x86_64__)
                asm volatile (
                    "movd %1, %0"
                    : "=x"(fval)
                    : "r"(ival)
                );
                #endif
                
                // More register pressure
                v6 = matrix[i][j] + v0;
                v7 = v6 * v1 - v2;
                v8 = v7 / (v3 + 1);
                v9 = v8 ^ v4;
                v10 = v9 | v5;
                
                // Floating point computations
                f1 = f0 * 1.1f + (float)v6;
                f2 = f1 - f0 * 0.5f;
                f3 = f2 * 2.0f + (float)v7;
                
                // Double precision
                d1 = d0 * 1.01 + (double)v8;
                d2 = d1 - d0 * 0.49;
                d3 = d2 * 1.5 + arr3d[idx1][idx2][idx3];
                
                // Character manipulations
                c1 = c0 + (char)i;
                c2 = c1 - (char)j;
                c3 = c2 * (char)v9;
                
                // Volatile assignments forcing stores
                sink = v10;
                volatile int vsink;
                vsink = v6 + v7 + v8;
                
                // Chain all variables together
                v11 = v10 + (int)f1 + (int)d1 + (int)c1;
                v12 = v11 * v0 - (int)f2 + (int)d2;
                v13 = v12 / (v1 + 1) ^ (int)c2;
                v14 = v13 | v2 + (int)f3;
                v15 = v14 & v3 * (int)d3;
                
                // More inline assembly with memory constraints
                int mem_temp;
                asm volatile (
                    "ldr %0, [%1]\n\t"
                    "add %0, %0, %2\n\t"
                    "str %0, [%1]"
                    : "=&r"(mem_temp), "+r"(ptr)
                    : "r"(v15)
                    : "memory"
                );
                
                // Function call with many register arguments
                v16 = helper_func(v0, v1, v2, v3, v4, v5, v6, v7);
                
                // Update checksum
                checksum += v16 + (int)(f1 + f2 + f3) + (int)(d1 + d2 + d3);
                
                // Rotate values to keep them all live
                v0 = v1; v1 = v2; v2 = v3; v3 = v4; v4 = v5;
                v5 = v6; v6 = v7; v7 = v8; v8 = v9; v9 = v10;
                v10 = v11; v11 = v12; v12 = v13; v13 = v14; v14 = v15;
                
                f0 = f1; f1 = f2; f2 = f3;
                d0 = d1; d1 = d2; d2 = d3;
                c0 = c1; c1 = c2; c2 = c3;
            }
            
            // Update pointer with complex computation
            int node_idx = (i * 29) % SIZE;
            current = &nodes[node_idx];
        }
        
        // Final volatile store with complex expression
        volatile double final_sink;
        final_sink = d0 + d1 + d2 + (double)v0 + (double)v1 + (double)v2;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

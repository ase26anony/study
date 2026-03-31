#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 128
#define DIM 16

struct DataNode {
    int data[SIZE];
    float fdata[SIZE];
    double ddata[SIZE];
    struct DataNode *next;
};

__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = 0;
    // Force register usage with inline asm
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
    // Initialize complex data structures
    int matrix1[DIM][DIM][DIM];
    double matrix2[DIM][DIM][DIM];
    struct DataNode nodes[4];
    volatile int sink;
    
    // Initialize data
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            for (int k = 0; k < DIM; k++) {
                matrix1[i][j][k] = i * 1000 + j * 100 + k;
                matrix2[i][j][k] = i * 0.1 + j * 0.01 + k * 0.001;
            }
        }
    }
    
    for (int n = 0; n < 4; n++) {
        for (int i = 0; i < SIZE; i++) {
            nodes[n].data[i] = n * SIZE + i;
            nodes[n].fdata[i] = (n * SIZE + i) * 0.5f;
            nodes[n].ddata[i] = (n * SIZE + i) * 0.25;
        }
        nodes[n].next = (n < 3) ? &nodes[n+1] : NULL;
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix1, matrix2, nodes) map(from: checksum)
    {
        // Create many local variables to consume registers
        register int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        register float f1, f2, f3, f4, f5, f6, f7, f8;
        register double d1, d2, d3, d4, d5, d6;
        register char c1, c2, c3, c4, c5;
        int temp1, temp2, temp3, temp4, temp5;
        float ftemp1, ftemp2, ftemp3;
        double dtemp1, dtemp2;
        
        // Initialize from mapped arrays with complex addressing
        v1 = matrix1[0][0][0];
        v2 = matrix1[1][1][1];
        v3 = matrix1[2][2][2];
        v4 = matrix1[3][3][3];
        
        f1 = (float)matrix2[0][0][0];
        f2 = (float)matrix2[1][1][1];
        f3 = (float)matrix2[2][2][2];
        f4 = (float)matrix2[3][3][3];
        
        d1 = matrix2[4][4][4];
        d2 = matrix2[5][5][5];
        
        // Pointer chain traversal
        struct DataNode *current = &nodes[0];
        c1 = (char)current->data[0];
        c2 = (char)current->fdata[1];
        c3 = (char)current->next->data[2];
        c4 = (char)current->next->next->data[3];
        c5 = (char)current->next->next->next->data[4];
        
        // Complex nested loop with register pressure
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                for (int k = 0; k < 8; k++) {
                    // Complex array indexing with multiple computations
                    int idx1 = (i * DIM + j * 2 + k) % SIZE;
                    int idx2 = (j * DIM + k * 3 + i) % SIZE;
                    int idx3 = (k * DIM + i * 4 + j) % SIZE;
                    
                    // Chain computations to keep variables live
                    v5 = matrix1[i][j][k] + v1;
                    v6 = matrix1[j][k][i] + v2;
                    v7 = matrix1[k][i][j] + v3;
                    v8 = matrix1[(i+j)%DIM][(j+k)%DIM][(k+i)%DIM] + v4;
                    
                    // Mixed type operations forcing conversions
                    ftemp1 = f1 + (float)v5;
                    ftemp2 = f2 + (float)v6;
                    ftemp3 = f3 + (float)v7;
                    
                    dtemp1 = d1 + (double)ftemp1;
                    dtemp2 = d2 + (double)ftemp2;
                    
                    // Inline assembly with register constraints
                    // Force general purpose register usage
                    asm volatile (
                        "add %0, %1, %2\n\t"
                        "mul %0, %0, %3"
                        : "=r"(temp1)
                        : "r"(v5), "r"(v6), "r"(v7)
                        : "cc"
                    );
                    
                    // Force memory operand
                    asm volatile (
                        "ldr %0, [%1]\n\t"
                        "add %0, %0, %2"
                        : "=r"(temp2)
                        : "r"(&matrix1[i][j][k]), "r"(temp1)
                        : "memory"
                    );
                    
                    // Mixed register class usage (simulate secondary reloads)
                    // This may require moving between register classes
                    int ival = temp1 + temp2;
                    float fval;
                    asm volatile (
                        "scvtf %s0, %w1"
                        : "=w"(fval)
                        : "r"(ival)
                    );
                    
                    // Force output reload with complex addressing
                    int *ptr = &current->data[idx1];
                    *ptr = temp1 + temp2;  // out operand with register pointer
                    
                    // Volatile store forcing output reload
                    sink = temp1 * temp2 + ival;
                    
                    // Complex struct member access with pointer chain
                    current->fdata[idx2] = ftemp1 + ftemp2 + fval;
                    current->next->ddata[idx3] = dtemp1 + dtemp2 + (double)fval;
                    
                    // More register pressure
                    v9 = v5 * v6 - v7 / (v8 + 1);
                    v10 = v6 * v7 - v8 / (v5 + 1);
                    
                    f5 = ftemp1 * 0.5f + f4;
                    f6 = ftemp2 * 0.25f + f3;
                    f7 = ftemp3 * 0.125f + f2;
                    f8 = fval * 2.0f + f1;
                    
                    d3 = dtemp1 * 0.1 + d2;
                    d4 = dtemp2 * 0.01 + d1;
                    d5 = (double)fval * 0.001 + d3;
                    d6 = d4 * d5 - d3 / (d2 + 1.0);
                    
                    c1 = (char)(v9 % 256);
                    c2 = (char)(v10 % 256);
                    c3 = (char)((v9 + v10) % 256);
                    c4 = (char)((v9 * v10) % 256);
                    c5 = (char)((v9 - v10) % 256);
                    
                    // Function call with many register arguments
                    temp3 = helper_func(v5, v6, v7, v8, v9, v10, temp1, temp2);
                    
                    // More complex addressing
                    matrix1[(i*2 + j) % DIM][(j*3 + k) % DIM][(k*4 + i) % DIM] = 
                        temp3 + current->data[(i+j+k) % SIZE];
                    
                    // Update checksum
                    checksum += v5 + v6 + v7 + v8 + v9 + v10 + 
                               (int)f5 + (int)f6 + (int)f7 + (int)f8 +
                               (int)d3 + (int)d4 + (int)d5 + (int)d6 +
                               c1 + c2 + c3 + c4 + c5;
                }
            }
        }
        
        // Final complex computation
        temp4 = 0;
        for (int i = 0; i < 16; i++) {
            // Pointer arithmetic forcing address computation
            int *base = &current->data[0];
            int offset = (i * 7) % SIZE;
            temp4 += *(base + offset);
            
            // Another inline asm with mixed constraints
            double dval;
            asm volatile (
                "fmov %d0, %w1\n\t"
                "fadd %d0, %d0, %d2"
                : "=w"(dval)
                : "r"(temp4), "w"(d6)
                :
            );
            
            // Force output to memory with computed index
            current->next->fdata[offset] = (float)dval;
        }
        
        temp5 = checksum + temp4;
        
        // Final volatile store
        sink = temp5;
        checksum = temp5;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

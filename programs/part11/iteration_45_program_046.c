#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 128
#define N_VARS 25

struct DataNode {
    int data[SIZE];
    double *next;
    volatile int flag;
};

__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, 
                float f, float g, double h, double i, char j) {
    volatile int sink;
    int result = (a * b) + (c - d) * e;
    result += (int)(f * g) + (int)(h / i) + j;
    sink = result;
    return result % 256;
}

int main() {
    // Initialize complex data structures
    int matrix1[SIZE][SIZE];
    double matrix2[SIZE][SIZE];
    struct DataNode nodes[SIZE];
    double *ptr_array[SIZE];
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * SIZE + j;
            matrix2[i][j] = (i * 0.1) + (j * 0.01);
        }
        nodes[i].next = (i < SIZE-1) ? &matrix2[i+1][0] : NULL;
        nodes[i].flag = i % 2;
        ptr_array[i] = &matrix2[i][i];
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix1, matrix2, nodes, ptr_array) \
                      map(from: checksum)
    {
        // Create many local variables to consume registers
        int var0, var1, var2, var3, var4, var5, var6, var7, var8, var9;
        int var10, var11, var12, var13, var14, var15, var16, var17, var18, var19;
        float fvar0, fvar1, fvar2, fvar3, fvar4;
        double dvar0, dvar1, dvar2, dvar3;
        char cvar0, cvar1, cvar2;
        volatile int vsink;
        volatile double dsink;
        
        // Initialize from mapped arrays with complex addressing
        var0 = matrix1[0][0];
        var1 = matrix1[SIZE/4][SIZE/4];
        var2 = matrix1[SIZE/2][SIZE/2];
        var3 = matrix1[3*SIZE/4][3*SIZE/4];
        
        // Complex pointer chain access
        double *dptr = nodes[0].next;
        if (dptr) {
            dvar0 = *dptr + matrix2[0][0];
        }
        
        // Nested loops with complex array indexing
        for (int i = 1; i < 8; i++) {
            for (int j = 1; j < 8; j++) {
                // Complex array indexing forcing address computation
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 19) % SIZE;
                int idx3 = (i * 29 + j * 31) % SIZE;
                
                // Force register pressure with many live variables
                var4 = matrix1[idx1][idx2];
                var5 = matrix1[idx2][idx3];
                var6 = matrix1[idx3][idx1];
                
                // Mixed type computations
                fvar0 = (float)var4 * 0.5f;
                fvar1 = (float)var5 * 0.25f;
                dvar1 = (double)var6 * 0.125;
                
                // Inline assembly with register constraints
                // Force specific register allocation conflicts
                asm volatile (
                    "add %0, %1, %2\n\t"
                    : "=r"(var7)
                    : "r"(var4), "r"(var5)
                    : "cc"
                );
                
                // Another asm with different constraints
                asm volatile (
                    "mul %0, %1, %2\n\t"
                    : "=r"(var8)
                    : "r"(var6), "r"(i)
                );
                
                // Floating point asm forcing FP register usage
                float ftemp;
                asm volatile (
                    "fmul %s0, %s1, %s2\n\t"
                    : "=w"(ftemp)
                    : "w"(fvar0), "w"(fvar1)
                );
                fvar2 = ftemp;
                
                // Mixed-type asm (potentially requiring secondary reloads)
                int itof;
                asm volatile (
                    "scvtf %s0, %w1\n\t"
                    : "=w"(dvar2)
                    : "r"(var7)
                );
                
                // Complex addressing with struct member access
                var9 = nodes[idx1].data[idx2] + nodes[idx2].data[idx3];
                
                // Pointer arithmetic forcing address reloads
                double *temp_ptr = ptr_array[idx1] + idx2;
                dvar3 = *temp_ptr + matrix2[idx2][idx3];
                
                // Chain computations to keep variables live
                var10 = var7 + var8 + var9;
                var11 = var10 * i - j;
                var12 = var11 / (j + 1);
                var13 = var12 | var9;
                var14 = var13 & 0xFF;
                
                // Mixed type expression forcing mode conversion
                fvar3 = (float)var14 + fvar2;
                dvar3 = dvar1 + dvar2 + (double)fvar3;
                
                // Volatile stores forcing output reloads
                vsink = var14;
                dsink = dvar3;
                
                // Assignment to array element with computed index
                // Forces out operand with complex addressing
                int store_idx = (i * 11 + j * 7) % SIZE;
                matrix1[store_idx][store_idx] = var14;
                
                // Complex pointer dereference assignment
                int *int_ptr = &matrix1[i][j];
                *int_ptr = var11 + var12;
                
                // More register pressure
                cvar0 = (char)(var14 & 0xFF);
                cvar1 = (char)(var13 & 0xFF);
                cvar2 = cvar0 + cvar1 + (char)i;
                
                var15 = var14 + cvar0;
                var16 = var15 * 2 - cvar1;
                var17 = var16 / (cvar2 + 1);
                var18 = var17 ^ var15;
                var19 = var18 & 0x7F;
                
                // Call helper function with many register arguments
                // Forces calling convention handling and potential reloads
                int func_result = helper_func(var14, var15, var16, var17, var18,
                                             fvar0, fvar1, dvar1, dvar2, cvar0);
                
                // Use all variables in final computation to keep them live
                checksum += var0 + var1 + var2 + var3 + var4 + var5 + var6 +
                           var7 + var8 + var9 + var10 + var11 + var12 + var13 +
                           var14 + var15 + var16 + var17 + var18 + var19 +
                           (int)fvar0 + (int)fvar1 + (int)fvar2 + (int)fvar3 +
                           (int)dvar0 + (int)dvar1 + (int)dvar2 + (int)dvar3 +
                           cvar0 + cvar1 + cvar2 + func_result;
                
                // Additional asm with memory constraint
                int mem_temp;
                asm volatile (
                    "ldr %0, [%1]\n\t"
                    "add %0, %0, #1\n\t"
                    "str %0, [%1]\n\t"
                    : "=&r"(mem_temp)
                    : "r"(&matrix1[i][j])
                    : "memory"
                );
            }
        }
        
        // Final complex computation using all variables
        int final = 0;
        for (int k = 0; k < 5; k++) {
            // Force address computation in loop
            int idx = (checksum + k * 7) % SIZE;
            final += nodes[idx].data[idx] + (int)matrix2[idx][idx];
            
            // More inline asm with constraints
            asm volatile (
                "madd %0, %1, %2, %3\n\t"
                : "=r"(var0)
                : "r"(var1), "r"(k), "r"(var2)
            );
        }
        checksum += final;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define NUM_VARS 25

struct DataStruct {
    int data[SIZE];
    float fdata[SIZE];
    double ddata[SIZE];
    char *ptr;
    int offset;
};

__attribute__((noinline))
int helper_function(int a, int b, float c, double d, char e, 
                    int f, int g, float h, double i, char j) {
    volatile int result = 0;
    // Force register usage with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("fadd %s0, %s1, %s2" : "=w"(c) : "w"(c), "w"(h));
    return result + (int)c + (int)d + e + f + g + (int)i + j;
}

int main() {
    // Initialize multi-dimensional arrays
    int matrix1[SIZE][SIZE];
    int matrix2[SIZE][SIZE];
    double dmatrix[SIZE][SIZE];
    float farr[SIZE * 2];
    char carr[SIZE * 3];
    
    struct DataStruct ds1, ds2;
    ds1.ptr = carr;
    ds1.offset = SIZE/2;
    ds2.ptr = carr + SIZE;
    ds2.offset = SIZE/4;
    
    // Initialize with some data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * SIZE + j;
            matrix2[i][j] = (i + j) % 256;
            dmatrix[i][j] = (double)(i * j) / 100.0;
        }
        farr[i] = (float)i / 3.0f;
        farr[i + SIZE] = (float)i / 5.0f;
        carr[i] = (char)(i % 128);
        carr[i + SIZE] = (char)((i + 64) % 128);
        carr[i + 2*SIZE] = (char)((i * 3) % 128);
        ds1.data[i] = i * 2;
        ds1.fdata[i] = (float)i * 1.5f;
        ds1.ddata[i] = (double)i * 2.5;
        ds2.data[i] = i * 3;
        ds2.fdata[i] = (float)i * 2.5f;
        ds2.ddata[i] = (double)i * 3.5;
    }
    
    int result = 0;
    
    #pragma omp target map(to: matrix1, matrix2, dmatrix, farr, carr, ds1, ds2) \
                       map(from: result)
    {
        // Declare many local variables to create register pressure
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        int var11, var12, var13, var14, var15, var16, var17, var18, var19, var20;
        float fvar1, fvar2, fvar3, fvar4, fvar5;
        double dvar1, dvar2, dvar3;
        char cvar1, cvar2, cvar3, cvar4;
        volatile int vsink1, vsink2, vsink3;
        volatile float vfsink;
        volatile double vdsink;
        
        // Initialize from mapped arrays with complex addressing
        var1 = matrix1[0][0];
        var2 = matrix2[ds1.offset % SIZE][ds2.offset % SIZE];
        var3 = ds1.data[ds1.offset] + ds2.data[ds2.offset];
        
        // Complex pointer chain access
        char *ptr1 = ds1.ptr + ds1.offset;
        char *ptr2 = ds2.ptr + ds2.offset;
        cvar1 = *ptr1;
        cvar2 = *ptr2;
        
        // Mixed type computations
        fvar1 = farr[var1 % (SIZE*2)];
        dvar1 = dmatrix[var2 % SIZE][var3 % SIZE];
        
        // Nested loops creating complex addressing and register pressure
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                // Complex index computation forcing address reloads
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 19) % SIZE;
                int idx3 = (i * 29 + j * 31) % SIZE;
                
                // Multi-dimensional array access with complex expression
                var4 = matrix1[idx1][idx2] + matrix2[idx2][idx3];
                
                // Struct member access with pointer chain
                var5 = ds1.data[idx1] + ds2.data[idx2];
                
                // Mixed type expression forcing mode conversions
                fvar2 = ds1.fdata[idx1] + ds2.fdata[idx2];
                dvar2 = ds1.ddata[idx1] + ds2.ddata[idx3];
                
                // Inline assembly with register constraints
                // Force general purpose register usage
                asm volatile ("add %0, %1, %2" : "=r"(var6) : "r"(var4), "r"(var5));
                
                // Force memory operand
                asm volatile ("ldr %0, [%1]" : "=r"(var7) : "r"(&matrix1[idx1][idx2]));
                
                // Mixed register class usage (simulating secondary reloads)
                // Move between general and floating-point registers
                int temp_int = var6 + var7;
                asm volatile ("fmov %s0, %w1" : "=w"(fvar3) : "r"(temp_int));
                
                // Another asm with multiple constraints
                float ftemp;
                asm volatile ("fcvt %s0, %w1" : "=w"(ftemp) : "r"(temp_int));
                fvar4 = fvar2 + ftemp;
                
                // Complex addressing for store (output reload)
                int store_idx = (idx1 + idx2 + idx3) % SIZE;
                matrix1[store_idx][store_idx % 16] = var6 + var7;
                
                // Volatile store forcing output reload
                vsink1 = var6;
                vfsink = fvar4;
                vdsink = dvar2;
                
                // Pointer dereference assignment
                int *int_ptr = &matrix2[store_idx][store_idx % 16];
                *int_ptr = var7 * 2;
                
                // Chain computations to keep variables live
                var8 = var6 + var7 + var4 + var5;
                var9 = var8 * 3 - var6;
                var10 = var9 / 2 + var7;
                
                // More mixed type operations
                fvar5 = fvar1 + fvar2 + fvar3 + fvar4;
                dvar3 = dvar1 + dvar2 + (double)fvar5;
                
                // Character operations
                cvar3 = carr[store_idx] + cvar1;
                cvar4 = carr[store_idx + SIZE] + cvar2;
                
                // More variables to increase pressure
                var11 = var8 + var9 + var10;
                var12 = var11 * (i + 1);
                var13 = var12 / (j + 1);
                var14 = var13 ^ var11;
                var15 = var14 & 0xFF;
                
                // Use builtin functions that may need specific registers
                var16 = __builtin_popcount(var15);
                var17 = __builtin_clz(var14);
                
                // More floating point operations
                float fvar6 = fvar5 * 2.0f;
                float fvar7 = fvar6 / 3.0f;
                
                // Double precision operations
                double dvar4 = dvar3 * 1.5;
                double dvar5 = dvar4 - 0.5;
                
                // More volatile stores
                vsink2 = var16;
                vsink3 = var17;
                
                // Complex assignment to array element
                dmatrix[store_idx % 8][store_idx % 16] = dvar5 + (double)var16;
                
                // Call helper function with many register arguments
                var18 = helper_function(var8, var9, fvar7, dvar5, cvar3,
                                       var10, var11, fvar6, dvar4, cvar4);
                
                // Accumulate result
                var19 = var18 + var12 + var13;
                var20 = var19 % 1000;
                
                // Final complex store with pointer arithmetic
                int *final_ptr = &matrix2[(i * j) % SIZE][(i + j) % SIZE];
                *final_ptr = var20;
            }
        }
        
        // Compute final checksum
        result = 0;
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                result += matrix1[i][j] + matrix2[i][j];
                result += (int)dmatrix[i][j];
            }
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}

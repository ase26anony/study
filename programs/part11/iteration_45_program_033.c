#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define VARS 25

typedef struct {
    int data[SIZE];
    double values[SIZE];
    char *ptr;
} DataStruct;

__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = 0;
    // Force register usage with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %0, %1" : "+r"(result) : "r"(c));
    asm volatile ("and %0, %0, %1" : "+r"(result) : "r"(d));
    asm volatile ("or %0, %0, %1" : "+r"(result) : "r"(e));
    asm volatile ("xor %0, %0, %1" : "+r"(result) : "r"(f));
    asm volatile ("sub %0, %0, %1" : "+r"(result) : "r"(g));
    asm volatile ("add %0, %0, %1" : "+r"(result) : "r"(h));
    return result;
}

int main() {
    // Initialize complex data structures
    DataStruct ds1, ds2;
    int matrix1[SIZE][SIZE];
    int matrix2[SIZE][SIZE];
    double dmatrix[SIZE][SIZE];
    char buffer[SIZE * SIZE];
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * SIZE + j;
            matrix2[i][j] = (i + j) * 3;
            dmatrix[i][j] = (i * 0.5) + (j * 0.25);
        }
        ds1.data[i] = i * 2;
        ds2.data[i] = i * 3;
        ds1.values[i] = i * 1.5;
        ds2.values[i] = i * 2.5;
    }
    
    ds1.ptr = buffer;
    ds2.ptr = buffer + SIZE * SIZE / 2;
    memset(buffer, 'A', sizeof(buffer));
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix1, matrix2, dmatrix, ds1, ds2) \
                      map(tofrom: checksum) map(to: buffer)
    {
        // Declare many local variables to create register pressure
        int var0, var1, var2, var3, var4, var5, var6, var7, var8, var9;
        int var10, var11, var12, var13, var14, var15, var16, var17, var18, var19;
        float fvar0, fvar1, fvar2, fvar3, fvar4;
        double dvar0, dvar1, dvar2, dvar3, dvar4;
        char cvar0, cvar1, cvar2, cvar3, cvar4;
        volatile int vsink;
        volatile double vdsink;
        
        // Initialize from mapped data with complex addressing
        var0 = matrix1[0][0];
        var1 = matrix2[1][1];
        var2 = ds1.data[10];
        var3 = ds2.data[20];
        var4 = buffer[0];
        var5 = buffer[SIZE];
        
        // Complex index calculations forcing address reloads
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                // Multi-dimensional array with complex index
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 19) % SIZE;
                
                // Force address computation in registers
                var6 = matrix1[idx1][idx2] + matrix2[idx2][idx1];
                var7 = matrix1[(i * j + idx1) % SIZE][(i + j * 3) % SIZE];
                
                // Struct member access with pointer chain
                var8 = ds1.data[(idx1 + idx2) % SIZE] + 
                       ds2.data[(idx1 * 2) % SIZE];
                
                // Mixed type operations forcing mode conversions
                fvar0 = (float)var6 * 1.5f;
                dvar0 = (double)var7 * 2.5;
                cvar0 = (char)(var8 & 0xFF);
                
                // Inline assembly with register constraints
                asm volatile ("add %0, %1, %2" : "=r"(var9) : "r"(var6), "r"(var7));
                asm volatile ("mul %0, %0, %1" : "+r"(var9) : "r"(var8));
                
                // Force output reload with volatile assignment
                vsink = var9;
                
                // More complex addressing with struct pointer
                int *data_ptr = ds1.data;
                double *val_ptr = ds1.values;
                
                // Pointer arithmetic forcing address reload
                var10 = *(data_ptr + idx1) + *(data_ptr + idx2);
                dvar1 = *(val_ptr + idx1) * *(val_ptr + idx2);
                
                // Mixed precision operations
                var11 = (int)dvar1 + var10;
                fvar1 = (float)dvar1 + (float)var10;
                
                // Chain many variables together
                var12 = var0 + var1 + var2 + var3 + var4 + var5;
                var13 = var6 * var7 - var8 + var9;
                var14 = var10 ^ var11 | var12 & var13;
                
                // Floating point operations in FP registers
                dvar2 = dvar0 * 1.7 + dvar1 * 2.3;
                fvar2 = fvar0 * 3.1f + fvar1 * 4.2f;
                
                // Convert between register classes
                int ival = (int)dvar2;
                double dval = (double)var14;
                
                // Secondary reload scenario: move between register classes
                #ifdef __aarch64__
                asm volatile ("fmov %s0, %w1" : "=w"(fvar3) : "r"(ival));
                asm volatile ("fmov %w0, %s1" : "=r"(var15) : "w"(fvar2));
                #elif __x86_64__
                asm volatile ("movd %1, %0" : "=x"(fvar3) : "r"(ival));
                asm volatile ("movd %1, %0" : "=r"(var15) : "x"(fvar2));
                #else
                fvar3 = (float)ival;
                var15 = (int)fvar2;
                #endif
                
                // More register pressure
                var16 = __builtin_popcount(var14) + var15;
                var17 = __builtin_ffs(var16) * var13;
                
                // Complex expression with many live variables
                var18 = (var12 * var13) / (var14 + 1) + 
                        (var15 << 2) - (var16 >> 1) | 
                        (var17 & 0xFFFF);
                
                // Force store to memory with computed index
                int store_idx = (i * 31 + j * 29) % SIZE;
                ds1.data[store_idx] = var18;
                
                // Volatile double store
                vdsink = dvar2 + fvar3;
                
                // Call helper function with many register arguments
                var19 = helper_func(var0, var1, var2, var3, var4, var5, var6, var7);
                
                // Update checksum
                checksum += var18 + var19 + (int)vdsink;
                
                // Rotate variables to extend live ranges
                var0 = var1; var1 = var2; var2 = var3; var3 = var4; var4 = var5;
                var5 = var6; var6 = var7; var7 = var8; var8 = var9; var9 = var10;
                var10 = var11; var11 = var12; var12 = var13; var13 = var14; var14 = var15;
                var15 = var16; var16 = var17; var17 = var18; var18 = var19;
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

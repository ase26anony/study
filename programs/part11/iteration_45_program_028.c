#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define VARS 20

typedef struct {
    int data[SIZE];
    double values[SIZE];
    char *ptr;
} DataStruct;

__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, float f, double g) {
    volatile int result;
    // Force register usage and potential reloads
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %1, %2" : "+r"(result) : "r"(c), "r"(d));
    return result + (int)(f * g);
}

int main() {
    // Initialize complex data structures
    DataStruct ds1, ds2;
    int matrix1[SIZE][SIZE];
    int matrix2[SIZE][SIZE];
    double arr3d[SIZE][SIZE][2];
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        ds1.data[i] = i * 2;
        ds2.data[i] = i * 3;
        ds1.values[i] = i * 1.5;
        ds2.values[i] = i * 2.5;
        
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * SIZE + j;
            matrix2[i][j] = (i * SIZE + j) * 2;
            
            for (int k = 0; k < 2; k++) {
                arr3d[i][j][k] = (i * j * k) * 1.1;
            }
        }
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: ds1, ds2, matrix1, matrix2, arr3d) map(from: checksum)
    {
        // Declare many local variables to create register pressure
        int var0, var1, var2, var3, var4, var5, var6, var7, var8, var9;
        int var10, var11, var12, var13, var14, var15, var16, var17, var18, var19;
        float fvar0, fvar1, fvar2, fvar3, fvar4;
        double dvar0, dvar1, dvar2, dvar3;
        char cvar0, cvar1, cvar2, cvar3;
        volatile int vsink;  // For forcing output reloads
        volatile double dsink;
        
        // Initialize from mapped data with complex addressing
        var0 = ds1.data[0];
        var1 = ds2.data[SIZE-1];
        var2 = matrix1[0][0];
        var3 = matrix2[SIZE/2][SIZE/2];
        fvar0 = (float)ds1.values[10];
        fvar1 = (float)ds2.values[20];
        dvar0 = arr3d[1][2][0];
        dvar1 = arr3d[3][4][1];
        cvar0 = (char)(var0 & 0xFF);
        cvar1 = (char)(var1 & 0xFF);
        
        // Complex nested loops with register pressure
        for (int i = 1; i < 8; i++) {
            for (int j = 1; j < 8; j++) {
                // Complex array indexing forcing address reloads
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 19) % SIZE;
                int idx3 = (i * 29 + j * 31) % SIZE;
                
                // Chain computations keeping many variables live
                var4 = matrix1[idx1][idx2] + matrix2[idx2][idx3];
                var5 = ds1.data[idx1] * ds2.data[idx2];
                var6 = var4 ^ var5;
                var7 = (var6 << 3) | (var6 >> 29);
                
                // Mixed type computations forcing mode conversions
                fvar2 = fvar0 * (float)var4 + fvar1;
                dvar2 = dvar0 * (double)var5 + dvar1;
                fvar3 = (float)dvar2 + fvar2;
                dvar3 = (double)fvar3 * dvar2;
                
                // Character computations
                cvar2 = (char)((var4 + var5) & 0xFF);
                cvar3 = cvar0 ^ cvar1 ^ cvar2;
                var8 = (int)cvar3 * 256 + (int)cvar2;
                
                // Inline assembly with register constraints
                // Force specific register usage
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %0, %0, %3"
                    : "=r"(var9)
                    : "r"(var6), "r"(var7), "r"(var8)
                    : "cc"
                );
                
                // More assembly with mixed constraints
                int tmp1, tmp2;
                asm volatile (
                    "mov %0, %1\n\t"
                    "add %0, %0, %2"
                    : "=r"(tmp1), "=r"(tmp2)
                    : "r"(var9), "0"(var4), "1"(var5)
                );
                
                // Force output reloads with volatile and pointer dereference
                vsink = tmp1 + tmp2;
                
                // Complex pointer arithmetic and struct access
                int *ptr1 = &ds1.data[idx1];
                int *ptr2 = &ds2.data[idx2];
                var10 = *ptr1 + *ptr2;
                
                // More complex addressing
                var11 = matrix1[(i * idx1 + j) % SIZE][(j * idx2 + i) % SIZE];
                var12 = matrix2[idx3][(idx1 + idx2) % SIZE];
                
                // Chain more computations
                var13 = var10 * var11 - var12;
                var14 = (var13 << 1) + (var13 >> 31);
                var15 = var14 ^ var9;
                
                // Floating point assembly (may trigger secondary reloads)
                float ftmp;
                asm volatile (
                    "fmul %s0, %s1, %s2"
                    : "=w"(ftmp)
                    : "w"(fvar2), "w"(fvar3)
                );
                
                // Mixed register class constraints
                int itmp;
                asm volatile (
                    "fmov %w0, %s1\n\t"
                    "add %w0, %w0, %w2"
                    : "=r"(itmp)
                    : "w"(ftmp), "r"(var15)
                );
                
                // Force store to memory with complex address
                int store_idx = (i * 11 + j * 7) % SIZE;
                ds2.data[store_idx] = itmp;
                
                // More variables to increase pressure
                var16 = helper_func(var13, var14, var15, itmp, var10, fvar3, dvar3);
                var17 = var16 * 3 - var11;
                var18 = (var17 & 0xFFFF) | ((var17 >> 16) << 16);
                var19 = var18 ^ var12;
                
                // Final volatile store forcing output reload
                dsink = dvar3 + (double)var19;
                
                // Update checksum
                checksum += var19 + (int)ftmp + itmp;
            }
        }
        
        // Additional computations outside loops to keep variables live
        int final1 = var0 + var1 + var2 + var3 + var4 + var5;
        int final2 = var6 + var7 + var8 + var9 + var10;
        int final3 = var11 + var12 + var13 + var14 + var15;
        int final4 = var16 + var17 + var18 + var19;
        
        // Complex expression with mixed types
        double final_d = dvar0 + dvar1 + dvar2 + dvar3;
        float final_f = fvar0 + fvar1 + fvar2 + fvar3 + fvar4;
        char final_c = cvar0 + cvar1 + cvar2 + cvar3;
        
        // Force more output reloads
        vsink = final1 + final2 + final3 + final4 + (int)final_d + (int)final_f + final_c;
        checksum += vsink;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

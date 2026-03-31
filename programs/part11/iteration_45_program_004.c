#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define N_VARS 20

typedef struct {
    int data[SIZE];
    double values[SIZE];
    char *ptr;
} DataStruct;

__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, int f, 
                float g, float h, double i, double j) {
    volatile int sink;
    sink = a + b + c + d + e + f;
    return sink + (int)(g + h + i + j);
}

int main() {
    // Initialize complex data structures
    int matrix1[SIZE][SIZE];
    int matrix2[SIZE][SIZE];
    double dmatrix[SIZE][SIZE];
    char buffer[SIZE * SIZE];
    DataStruct ds[4];
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * SIZE + j;
            matrix2[i][j] = (i + j) % SIZE;
            dmatrix[i][j] = i * 0.5 + j * 0.3;
        }
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < SIZE; j++) {
            ds[i].data[j] = i * 100 + j;
            ds[i].values[j] = i * 0.1 + j * 0.01;
        }
        ds[i].ptr = buffer + i * SIZE;
    }
    
    int result = 0;
    
    #pragma omp target map(to: matrix1, matrix2, dmatrix, ds) \
                       map(tofrom: result) \
                       map(tofrom: buffer)
    {
        // Create register pressure with many local variables
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        int var11, var12, var13, var14, var15;
        float fvar1, fvar2, fvar3, fvar4, fvar5;
        double dvar1, dvar2, dvar3, dvar4;
        char cvar1, cvar2, cvar3, cvar4;
        volatile int vsink1, vsink2, vsink3;
        volatile double dsink1;
        
        // Initialize from mapped arrays with complex addressing
        var1 = matrix1[0][0];
        var2 = matrix2[SIZE-1][SIZE-1];
        var3 = ds[0].data[var1 % SIZE];
        var4 = ds[1].data[var2 % SIZE];
        
        // Complex pointer arithmetic
        char *ptr1 = ds[0].ptr + var1;
        char *ptr2 = ds[1].ptr + var2;
        char *ptr3 = ds[2].ptr + (var1 + var2) % SIZE;
        
        // Nested loops creating live range interference
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                // Complex array indexing forcing address reloads
                int idx1 = (i * 37 + j * 13) % SIZE;
                int idx2 = (i * 19 + j * 7) % SIZE;
                int idx3 = (i * 23 + j * 11) % SIZE;
                
                // Multi-dimensional array access with complex expressions
                var5 = matrix1[idx1][idx2] + matrix2[idx2][idx3];
                var6 = matrix1[idx3][idx1] - matrix2[idx1][idx2];
                
                // Struct member access with pointer chain
                var7 = ds[i % 4].data[idx1] + ds[j % 4].data[idx2];
                var8 = ds[(i + j) % 4].data[idx3];
                
                // Floating point computations
                fvar1 = (float)dmatrix[idx1][idx2];
                fvar2 = (float)dmatrix[idx2][idx3];
                dvar1 = dmatrix[idx1][idx2] * 1.5;
                dvar2 = dmatrix[idx2][idx3] * 2.5;
                
                // Mixed type operations forcing mode conversions
                var9 = (int)fvar1 + (int)dvar1 + var5;
                var10 = (int)fvar2 + (int)dvar2 + var6;
                
                // Inline assembly with register constraints
                // Force register conflicts
                asm volatile (
                    "add %0, %1, %2\n\t"
                    : "=r"(var11)
                    : "r"(var7), "r"(var8)
                    : "cc"
                );
                
                // More inline assembly with different constraints
                asm volatile (
                    "mul %0, %1, %2\n\t"
                    : "=r"(var12)
                    : "r"(var9), "r"(var10)
                );
                
                // Assembly forcing specific register class usage
                // Simulating secondary reload scenario
                int temp_int = var11 + var12;
                float temp_float;
                asm volatile (
                    "fmov s0, %w1\n\t"
                    "fcvt d0, s0\n\t"
                    "fmov %0, d0\n\t"
                    : "=w"(dvar3)
                    : "r"(temp_int)
                    : "d0", "s0"
                );
                
                // Another assembly with memory constraint
                asm volatile (
                    "ldr %0, [%1]\n\t"
                    : "=r"(var13)
                    : "r"(ptr1 + idx1)
                );
                
                // Chain computations to keep variables live
                var14 = var11 * var12 + var13;
                var15 = var14 - var9 + var10;
                
                // Force output reloads with volatile stores
                vsink1 = var14;
                vsink2 = var15;
                dsink1 = dvar3;
                
                // Assignments to dereferenced pointers
                *(ptr2 + idx2) = (char)(var14 % 256);
                *(ptr3 + idx3) = (char)(var15 % 256);
                
                // Complex assignment to array element
                buffer[(i * 32 + j) % (SIZE * SIZE)] = 
                    (char)((var14 + var15) % 256);
                
                // Mixed char operations
                cvar1 = (char)var14;
                cvar2 = (char)var15;
                cvar3 = cvar1 + cvar2;
                cvar4 = cvar3 - (char)(i + j);
                
                // Use register keyword to hint allocation
                register int reg_var1 asm("x19") = var14;
                register int reg_var2 asm("x20") = var15;
                
                // Force function call with many register arguments
                if ((i + j) % 16 == 0) {
                    int call_result = helper_func(
                        var14, var15, var11, var12, var13,
                        reg_var1, fvar1, fvar2, dvar1, dvar2
                    );
                    vsink3 = call_result;
                }
                
                // Update pointers with arithmetic
                ptr1 += (var14 % 8);
                ptr2 += (var15 % 8);
                ptr3 += ((var14 + var15) % 8);
            }
        }
        
        // Final computation for checksum
        result = var1 + var2 + var3 + var4 + var14 + var15 + 
                 (int)fvar1 + (int)fvar2 + (int)dvar1 + (int)dvar2 +
                 cvar1 + cvar2 + cvar3 + cvar4;
    }
    
    printf("Result: %d\n", result);
    return 0;
}

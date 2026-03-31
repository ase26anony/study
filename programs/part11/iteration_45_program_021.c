#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define NUM_VARS 25

struct DataStruct {
    int data[SIZE];
    float fdata[SIZE];
    double ddata[SIZE];
    char cdata[SIZE];
    struct DataStruct *next;
};

__attribute__((noinline))
int helper_function(int a, int b, int c, float d, double e, char f, int g, int h) {
    volatile int result;
    // Force register usage with inline asm
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c)
        : "cc"
    );
    return result + (int)d + (int)e + f + g + h;
}

int main() {
    // Initialize complex data structures
    int matrix1[SIZE][SIZE];
    int matrix2[SIZE][SIZE];
    float fmatrix[SIZE][SIZE];
    double dmatrix[SIZE][SIZE];
    struct DataStruct ds1, ds2;
    struct DataStruct *ptr1 = &ds1;
    struct DataStruct *ptr2 = &ds2;
    
    // Link structures
    ds1.next = &ds2;
    ds2.next = &ds1;
    
    // Initialize with some data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * SIZE + j;
            matrix2[i][j] = i * SIZE - j;
            fmatrix[i][j] = i * 0.5f + j * 0.25f;
            dmatrix[i][j] = i * 0.125 + j * 0.0625;
        }
        ds1.data[i] = i;
        ds1.fdata[i] = i * 1.5f;
        ds1.ddata[i] = i * 2.5;
        ds1.cdata[i] = i % 128;
        
        ds2.data[i] = SIZE - i;
        ds2.fdata[i] = (SIZE - i) * 1.5f;
        ds2.ddata[i] = (SIZE - i) * 2.5;
        ds2.cdata[i] = (SIZE - i) % 128;
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix1, matrix2, fmatrix, dmatrix, ds1, ds2) \
                      map(tofrom: checksum)
    {
        // Declare many local variables to create register pressure
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        int var11, var12, var13, var14, var15, var16, var17, var18, var19, var20;
        float fvar1, fvar2, fvar3, fvar4, fvar5;
        double dvar1, dvar2, dvar3, dvar4, dvar5;
        char cvar1, cvar2, cvar3, cvar4, cvar5;
        volatile int vsink1, vsink2, vsink3;
        volatile float fvsink;
        volatile double dvsink;
        
        // Initialize from mapped arrays with complex addressing
        var1 = matrix1[0][0];
        var2 = matrix2[0][0];
        fvar1 = fmatrix[0][0];
        dvar1 = dmatrix[0][0];
        cvar1 = ds1.cdata[0];
        
        // Complex nested loops with register pressure
        for (int i = 1; i < 8; i++) {
            for (int j = 1; j < 8; j++) {
                // Complex array indexing with multiple computations
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 19) % SIZE;
                int idx3 = (i * 29 + j * 31) % SIZE;
                
                // Chain computations across many variables
                var3 = matrix1[idx1][idx2] + matrix2[idx2][idx1];
                var4 = var3 * 2 - var1;
                var5 = var4 / 3 + var2;
                
                // Mixed type computations forcing conversions
                fvar2 = fmatrix[idx1][idx2] + var5;
                fvar3 = fvar1 * 2.5f - fvar2;
                
                dvar2 = dmatrix[idx2][idx1] + fvar3;
                dvar3 = dvar1 * 1.75 - dvar2;
                
                // Complex pointer chain access
                var6 = ptr1->data[idx3] + ptr1->next->data[idx1];
                var7 = var6 * ptr2->data[idx2];
                
                // More variable chaining
                var8 = var7 + var3 - var4;
                var9 = var8 * var5;
                var10 = var9 / (var6 + 1);
                
                // Inline assembly with register constraints
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %0, %0, %3"
                    : "=r"(var11)
                    : "r"(var8), "r"(var9), "r"(var10)
                    : "cc"
                );
                
                // Another asm with different constraints
                asm volatile (
                    "mul %0, %1, %2"
                    : "=r"(var12)
                    : "r"(var11), "r"(var7)
                );
                
                // Force output reload with volatile store
                vsink1 = var12;
                
                // Complex addressing for store
                matrix1[(i + j) % SIZE][(i * j) % SIZE] = var12;
                
                // More mixed computations
                fvar4 = fvar3 + var12;
                dvar4 = dvar3 * fvar4;
                
                // Struct member access with pointer arithmetic
                cvar2 = ptr1->cdata[idx1] + ptr2->cdata[idx2];
                cvar3 = cvar1 * cvar2;
                
                var13 = var12 + cvar3;
                var14 = var13 - var11;
                
                // Secondary reload trigger: move between register classes
                // This may require secondary reloads on some architectures
                int ival = var14;
                float fval;
                asm volatile (
                    #ifdef __aarch64__
                    "fmov %s0, %w1"
                    #elif defined(__x86_64__)
                    "movd %1, %0"
                    #else
                    "mov %1, %0"
                    #endif
                    : "=w"(fval)
                    : "r"(ival)
                );
                
                fvsink = fval;
                
                // More variables to increase pressure
                var15 = var14 + i * j;
                var16 = var15 - var13;
                var17 = var16 * var14;
                var18 = var17 / (var15 + 1);
                var19 = var18 + j;
                var20 = var19 * i;
                
                // Force output to array with complex index
                int store_idx = (i * 31 + j * 37) % SIZE;
                ptr2->data[store_idx] = var20;
                
                // Volatile double store
                dvsink = dvar4;
                
                // Call helper function with many register arguments
                var1 = helper_function(var1, var2, var3, fvar1, dvar1, cvar1, var4, var5);
                
                // Update checksum
                checksum += var20 + (int)fval + (int)dvar4;
                
                // Rotate variables to extend live ranges
                int tmp = var1;
                var1 = var2;
                var2 = var3;
                var3 = var4;
                var4 = var5;
                var5 = var6;
                var6 = var7;
                var7 = var8;
                var8 = var9;
                var9 = var10;
                var10 = tmp;
                
                float ftmp = fvar1;
                fvar1 = fvar2;
                fvar2 = fvar3;
                fvar3 = fvar4;
                fvar4 = ftmp;
                
                double dtmp = dvar1;
                dvar1 = dvar2;
                dvar2 = dvar3;
                dvar3 = dvar4;
                dvar4 = dtmp;
                
                char ctmp = cvar1;
                cvar1 = cvar2;
                cvar2 = cvar3;
                cvar3 = cvar4;
                cvar4 = ctmp;
            }
        }
        
        // Final volatile stores
        vsink2 = var20;
        vsink3 = checksum;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

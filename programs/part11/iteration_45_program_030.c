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

// Helper function to force register usage
__attribute__((noinline))
int compute_helper(int a, int b, int c, float d, double e, char f) {
    volatile int result;
    // Force register pressure with inline asm
    asm volatile (
        "add %w0, %w1, %w2\n\t"
        "add %w0, %w0, %w3\n\t"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c)
        : "cc"
    );
    return result + (int)d + (int)e + (int)f;
}

int main() {
    // Initialize arrays and structs
    int matrix1[SIZE][SIZE];
    int matrix2[SIZE][SIZE];
    double dmatrix[SIZE][SIZE];
    struct DataStruct ds1, ds2;
    volatile int sink = 0;
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * SIZE + j;
            matrix2[i][j] = i * SIZE - j;
            dmatrix[i][j] = (double)(i * j) / 100.0;
        }
        ds1.data[i] = i * 2;
        ds1.fdata[i] = i * 3.14f;
        ds1.ddata[i] = i * 2.71828;
        ds2.data[i] = i * 3;
        ds2.fdata[i] = i * 1.414f;
        ds2.ddata[i] = i * 1.61803;
    }
    ds1.offset = 17;
    ds2.offset = 23;
    
    int result = 0;
    
    #pragma omp target map(to: matrix1, matrix2, dmatrix, ds1, ds2) \
                      map(tofrom: result) map(from: sink)
    {
        // Declare many local variables to create register pressure
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        int var11, var12, var13, var14, var15, var16, var17, var18, var19, var20;
        float fvar1, fvar2, fvar3, fvar4, fvar5;
        double dvar1, dvar2, dvar3, dvar4, dvar5;
        char cvar1, cvar2, cvar3, cvar4, cvar5;
        volatile int vsink1, vsink2, vsink3;
        
        // Initialize from mapped arrays with complex indexing
        var1 = matrix1[0][0];
        var2 = matrix2[0][0];
        fvar1 = ds1.fdata[0];
        dvar1 = ds1.ddata[0];
        cvar1 = (char)ds1.data[0];
        
        // Complex nested loops with register pressure
        for (int i = 1; i < 8; i++) {
            for (int j = 1; j < 8; j++) {
                // Complex array indexing forcing address reloads
                int idx1 = (i * 13 + j * 7) % SIZE;
                int idx2 = (i * 17 + j * 11) % SIZE;
                int idx3 = (i * 19 + j * 13) % SIZE;
                
                // Chain computations creating long live ranges
                var3 = matrix1[idx1][idx2] + matrix2[idx2][idx1];
                var4 = var3 * 2 - var1;
                var5 = var4 / 3 + var2;
                
                // Mixed type operations forcing mode conversions
                fvar2 = (float)var5 + fvar1 * 2.0f;
                dvar2 = (double)fvar2 + dvar1 * 1.5;
                
                // Complex struct member access with pointer arithmetic
                var6 = ds1.data[idx3] + ds2.data[(idx1 + idx2) % SIZE];
                var7 = var6 + ds1.offset * 2 - ds2.offset;
                
                // Multi-dimensional array with complex expression
                var8 = matrix1[(i * 11 + j * 3) % SIZE][(i * 5 + j * 7) % SIZE];
                var9 = matrix2[(i * 7 + j * 11) % SIZE][(i * 13 + j * 17) % SIZE];
                
                // Force output reloads with volatile assignments
                vsink1 = var7 + var8 - var9;
                
                // Inline assembly with register constraints
                asm volatile (
                    "add %w0, %w1, %w2\n\t"
                    "sub %w0, %w0, %w3\n\t"
                    : "=r"(var10)
                    : "r"(var7), "r"(var8), "r"(var9)
                    : "cc"
                );
                
                // More mixed operations
                fvar3 = fvar2 + (float)var10 * 0.5f;
                dvar3 = dvar2 + (double)fvar3 * 0.25;
                
                // Force secondary reloads with specific register class constraints
                #ifdef __aarch64__
                // Move between general and FP registers on AArch64
                asm volatile (
                    "fmov %s0, %w1\n\t"
                    : "=w"(fvar4)
                    : "r"(var10)
                );
                #else
                // Generic version using 'r' and 'm' constraints
                asm volatile (
                    "mov %0, %1\n\t"
                    : "=r"(var11)
                    : "m"(var10)
                );
                fvar4 = (float)var11;
                #endif
                
                // Complex expression chain
                var12 = var10 * 3 + var7 / 2;
                var13 = var12 - var8 + var9;
                var14 = var13 * 2 % 256;
                cvar2 = (char)var14;
                
                // More volatile assignments forcing output reloads
                vsink2 = (int)cvar2 + var13;
                
                // Function call forcing argument passing in registers
                var15 = compute_helper(var10, var12, var13, fvar3, dvar3, cvar2);
                
                // Complex assignment to array element with computed index
                int store_idx = (i * 23 + j * 29) % (SIZE - 1);
                matrix1[store_idx][store_idx + 1] = var15;
                
                // More computations
                var16 = var15 + var10 - var12;
                fvar5 = (float)var16 * 1.1f + fvar4;
                dvar4 = (double)fvar5 * 0.9 + dvar3;
                
                // Final volatile sink
                vsink3 = (int)dvar4 + var16;
                
                // Update result
                result += var15 + var16 + (int)fvar5 + (int)dvar4;
            }
        }
        
        // Additional register pressure outside loops
        var17 = var1 + var2 + var3 + var4 + var5;
        var18 = var6 + var7 + var8 + var9 + var10;
        var19 = var11 + var12 + var13 + var14 + var15;
        var20 = var16 + var17 + var18 + var19;
        
        // Complex pointer arithmetic and dereference simulation
        int *ptr1 = &matrix1[0][0];
        int *ptr2 = &matrix2[0][0];
        for (int k = 0; k < 16; k++) {
            // Force address computation in registers
            int offset = k * 7;
            int val1 = *(ptr1 + offset);
            int val2 = *(ptr2 + offset * 2);
            
            // Inline asm with memory constraint
            asm volatile (
                "add %w0, %w1, %w2\n\t"
                : "=r"(var20)
                : "r"(val1), "r"(val2)
            );
            
            // Assign to volatile
            sink = var20;
        }
    }
    
    printf("Result: %d\n", result);
    printf("Sink: %d\n", sink);
    
    return 0;
}

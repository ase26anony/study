#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define VARS 25

typedef struct {
    int data[SIZE];
    double *matrix;
    volatile int counter;
} ComplexStruct;

__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, 
                float f, float g, double h, double i, char j) {
    volatile int result = 0;
    // Force register usage with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %0, %3" : "+r"(result) : "r"(c), "r"(d));
    return result + (int)f + (int)g + (int)h + (int)i + j;
}

int main() {
    // Initialize complex data structures
    int multi_dim[SIZE][SIZE];
    double matrix[SIZE][SIZE];
    ComplexStruct cs1, cs2;
    int *ptr_array[SIZE];
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            multi_dim[i][j] = i * SIZE + j;
            matrix[i][j] = (i * 0.1) + (j * 0.01);
        }
        cs1.data[i] = i * 2;
        cs2.data[i] = i * 3;
        ptr_array[i] = &cs1.data[i];
    }
    
    cs1.matrix = &matrix[0][0];
    cs2.matrix = &matrix[SIZE/2][SIZE/2];
    
    int result = 0;
    
    #pragma omp target map(to: multi_dim, matrix, cs1, cs2, ptr_array) \
                      map(tofrom: result) \
                      map(alloc: cs1.matrix[0:SIZE*SIZE/2])
    {
        // Declare many local variables to create register pressure
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        int var11, var12, var13, var14, var15, var16, var17, var18, var19, var20;
        float fvar1, fvar2, fvar3, fvar4, fvar5;
        double dvar1, dvar2, dvar3, dvar4;
        char cvar1, cvar2, cvar3;
        volatile int vsink1, vsink2, vsink3;
        volatile double dsink1, dsink2;
        
        // Initialize from mapped arrays with complex addressing
        var1 = multi_dim[0][0];
        var2 = multi_dim[SIZE-1][SIZE-1];
        var3 = cs1.data[var1 % SIZE];
        var4 = cs2.data[var2 % SIZE];
        
        // Complex pointer arithmetic and addressing
        int *ptr1 = &cs1.data[0];
        int *ptr2 = &cs2.data[SIZE/2];
        double *dptr1 = cs1.matrix;
        double *dptr2 = cs2.matrix;
        
        // Nested loops creating complex address computations
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                // Complex array indexing with multiple computations
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 19) % SIZE;
                int idx3 = (i * 29 + j * 31) % SIZE;
                
                // Chain computations keeping many variables live
                var5 = multi_dim[idx1][idx2] + ptr_array[idx3][0];
                var6 = var5 * var1 - var2;
                var7 = var6 / (var3 + 1) + var4;
                
                // Mixed type computations forcing mode conversions
                fvar1 = (float)var5 * 0.5f + (float)var6 * 0.25f;
                fvar2 = (float)var7 * 0.33f + fvar1;
                dvar1 = (double)fvar2 * 1.5 + matrix[idx1][idx2];
                
                // Inline assembly with register constraints
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "mul %0, %0, %3"
                    : "=r"(var8) 
                    : "r"(var5), "r"(var6), "r"(var7)
                    : "cc"
                );
                
                // More complex addressing with struct members
                var9 = cs1.data[(i * j) % SIZE] + cs2.data[(i + j) % SIZE];
                var10 = ptr1[idx1] + ptr2[idx2];
                
                // Force output reloads with volatile and pointer derefs
                vsink1 = var8 + var9;
                *(&cs1.data[0] + idx1) = var10;  // Complex base+offset store
                
                // Secondary reload triggers: mixed register classes
                int temp_int;
                double temp_double;
                asm volatile (
                    "fcvt %s0, %w1\n\t"  // Convert int to float (AArch64 style)
                    : "=w"(fvar3) 
                    : "r"(var8)
                );
                
                // Another asm with specific constraints
                asm volatile (
                    "mov %0, %1\n\t"
                    : "=r"(var11)
                    : "r"(var9)
                );
                
                // Chain more computations
                var12 = var8 + var9 + var10 + var11;
                var13 = var12 * i - j;
                var14 = var13 / (var1 + 1);
                
                // Floating point operations
                fvar4 = fvar1 + fvar2 + fvar3;
                dvar2 = dvar1 + (double)fvar4;
                
                // Complex store with computed index
                int store_idx = (i * 31 + j * 37) % SIZE;
                multi_dim[store_idx][(i + j) % SIZE] = var12 + var13;
                
                // More volatile sinks
                dsink1 = dvar1 + dvar2;
                vsink2 = var14;
                
                // Call helper function with many register arguments
                var15 = helper_func(var5, var6, var7, var8, var9,
                                   fvar1, fvar2, dvar1, dvar2, (char)i);
                
                // Final computation chain
                var16 = var15 + var12 + var13;
                var17 = var16 * 2 - var14;
                var18 = var17 / (var3 + 2);
                var19 = var18 + var11;
                var20 = var19 * 3;
                
                // More complex addressing for store
                double *complex_dptr = dptr1 + (i * SIZE + j);
                *complex_dptr = dvar1 + dvar2 + (double)var20;
                
                // Final sink
                vsink3 = var20;
                dsink2 = *complex_dptr;
                
                // Accumulate result
                result += var20 + (int)fvar4 + (int)dvar2;
            }
        }
        
        // Additional register pressure outside loops
        cvar1 = (char)(result & 0xFF);
        cvar2 = (char)((result >> 8) & 0xFF);
        cvar3 = cvar1 + cvar2;
        
        // More inline asm with constraints
        int final_result;
        asm volatile (
            "add %0, %1, %2\n\t"
            "add %0, %0, %3"
            : "=r"(final_result)
            : "r"(result), "r"(var20), "r"(cvar3)
        );
        
        result = final_result;
    }
    
    printf("Result: %d\n", result);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 128
#define NUM_VARS 25

struct Data {
    int values[SIZE];
    double fp_values[SIZE];
    char *ptr;
    int offset;
};

// Force no inlining to create function call pressure
__attribute__((noinline)) 
int helper_func(int a, int b, int c, int d, int e, 
                float f, float g, double h, double i) {
    volatile int result;
    // Force register usage with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("fadd %s0, %s1, %s2" : "=w"(f) : "w"(f), "w"(g));
    return result + (int)(f + h + i);
}

int main() {
    // Initialize complex data structures
    int matrix[SIZE][SIZE];
    double arr3d[SIZE][SIZE][SIZE];
    struct Data data_arr[SIZE];
    volatile int sink; // For forcing output reloads
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
        data_arr[i].ptr = (char*)matrix + i * SIZE * sizeof(int);
        data_arr[i].offset = i * 7 % SIZE;
        for (int k = 0; k < SIZE; k++) {
            data_arr[i].values[k] = i * k;
            data_arr[i].fp_values[k] = i * k * 0.5;
        }
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, data_arr, arr3d) map(from: checksum)
    {
        // Declare many local variables to create register pressure
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        int var11, var12, var13, var14, var15, var16, var17, var18;
        float fvar1, fvar2, fvar3, fvar4, fvar5;
        double dvar1, dvar2, dvar3, dvar4;
        char cvar1, cvar2, cvar3;
        register int reg_var1 asm ("r8"); // Hint specific register
        register int reg_var2 asm ("r9");
        
        // Initialize from mapped arrays with complex addressing
        var1 = matrix[0][0];
        var2 = matrix[SIZE/2][SIZE/2];
        var3 = data_arr[0].values[0];
        var4 = data_arr[SIZE/4].values[SIZE/4];
        
        // Complex nested loops with register pressure
        for (int i = 0; i < SIZE/8; i++) {
            for (int j = 0; j < SIZE/8; j++) {
                // Complex array indexing forcing address reloads
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 19) % SIZE;
                int idx3 = (i * 29 + j * 31) % SIZE;
                
                // Chain many computations to keep variables live
                var5 = matrix[idx1][idx2] + var1;
                var6 = matrix[idx2][idx3] * var2;
                var7 = data_arr[idx1].values[idx2] - var3;
                var8 = data_arr[idx3].values[idx1] / (var4 + 1);
                
                // Pointer arithmetic forcing address computation
                char *ptr1 = data_arr[i].ptr + j * sizeof(int);
                char *ptr2 = data_arr[j].ptr + i * sizeof(int);
                
                // Mixed type computations
                fvar1 = (float)var5 * 0.5f + (float)var6 * 0.25f;
                fvar2 = (float)var7 / 3.0f - (float)var8 * 0.1f;
                dvar1 = (double)fvar1 * 2.0 + (double)var5;
                dvar2 = (double)fvar2 * 3.0 - (double)var6;
                
                // More variables to increase pressure
                var9 = var5 ^ var6;
                var10 = var7 | var8;
                var11 = var9 & var10;
                var12 = var11 << 2;
                var13 = var12 >> 1;
                var14 = ~var13;
                
                // Inline assembly with register constraints
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %3, %4, %5"
                    : "=r"(var15), "=r"(var16)
                    : "r"(var9), "r"(var10), "r"(var11), "r"(var12)
                    : "cc"
                );
                
                // Force output reloads with volatile and pointer derefs
                sink = var15 + var16;
                *((int*)ptr1) = var15;  // Destination requires address reload
                *((int*)ptr2) = var16;
                
                // Complex 3D array access
                dvar3 = arr3d[idx1][idx2][idx3] + dvar1;
                dvar4 = arr3d[idx3][idx1][idx2] * dvar2;
                
                // Mixed register class usage (forcing secondary reloads)
                int ival = (int)dvar3;
                float fval;
                // Move between register classes (if supported by arch)
                asm volatile (
                    "fmov %s0, %w1"
                    : "=w"(fval)
                    : "r"(ival)
                );
                
                // More arithmetic chains
                var17 = var15 * var16 + ival;
                var18 = var17 - (int)fval;
                
                // Character operations with mode changes
                cvar1 = (char)(var17 & 0xFF);
                cvar2 = (char)(var18 & 0xFF);
                cvar3 = cvar1 + cvar2;
                
                // Function call with many register arguments
                reg_var1 = helper_func(var15, var16, var17, var18, ival,
                                      fvar1, fvar2, dvar3, dvar4);
                
                // Update checksum
                checksum += var15 + var16 + reg_var1 + cvar3;
                
                // Rotate variables to extend live ranges
                var1 = var15;
                var2 = var16;
                var3 = var17;
                var4 = var18;
                fvar1 = fval;
                dvar1 = dvar3;
                dvar2 = dvar4;
            }
        }
        
        // Final complex expression with many live variables
        int final_result = var1 + var2 + var3 + var4 + var5 + var6 + 
                          var7 + var8 + var9 + var10 + var11 + var12 +
                          var13 + var14 + var15 + var16 + var17 + var18 +
                          (int)fvar1 + (int)fvar2 + (int)dvar1 + (int)dvar2 +
                          cvar1 + cvar2 + cvar3 + reg_var1 + reg_var2;
        
        checksum += final_result;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

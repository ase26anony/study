#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define SIZE 128
#define NUM_VARS 25

typedef struct {
    int data[16];
    double values[8];
    char *ptr;
} ComplexStruct;

__attribute__((noinline))
int helper_function(int a, int b, int c, int d, int e, float f, double g) {
    volatile int result;
    // Force register usage with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %1, %2" : "+r"(result) : "r"(c), "r"(d));
    return result + (int)(f * g) + e;
}

int main() {
    // Initialize complex data structures
    int matrix[SIZE][SIZE];
    double big_array[SIZE * 2][SIZE];
    ComplexStruct structs[SIZE];
    volatile int sink = 0;
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
            big_array[i][j] = (i + j) * 0.5;
        }
        for (int k = 0; k < 16; k++) {
            structs[i].data[k] = i * 16 + k;
        }
        for (int k = 0; k < 8; k++) {
            structs[i].values[k] = i * 0.125 + k * 0.5;
        }
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, big_array, structs) map(from: checksum)
    {
        // Declare many local variables to create register pressure
        register int var0, var1, var2, var3, var4, var5, var6, var7, var8, var9;
        float fvar0, fvar1, fvar2, fvar3, fvar4;
        double dvar0, dvar1, dvar2, dvar3, dvar4;
        char cvar0, cvar1, cvar2, cvar3;
        volatile int vsink;
        volatile double dsink;
        
        // Initialize from mapped arrays with complex addressing
        var0 = matrix[0][0];
        var1 = matrix[SIZE/4][SIZE/2];
        var2 = matrix[SIZE/2][SIZE/4];
        
        // Complex array indexing forcing address computation
        for (int i = 1; i < SIZE/8; i++) {
            for (int j = 1; j < SIZE/8; j++) {
                // Complex addressing modes
                int idx1 = (i * (SIZE/2) + j * 3) % SIZE;
                int idx2 = (j * (SIZE/4) + i * 5) % SIZE;
                int idx3 = (i * j * 7) % SIZE;
                
                // Load with complex addressing - forces address reloads
                var3 = matrix[idx1][idx2];
                var4 = matrix[idx2][idx3];
                var5 = matrix[idx3][idx1];
                
                // Struct member access with pointer chain
                var6 = structs[i].data[j % 16];
                var7 = structs[j].data[i % 16];
                
                // Mixed type computations
                fvar0 = (float)big_array[i][j];
                fvar1 = (float)big_array[j][i];
                dvar0 = big_array[idx1][idx2];
                dvar1 = big_array[idx2][idx3];
                
                // Inline assembly with register constraints
                // Force specific register usage
                asm volatile ("add %0, %1, %2" : "=r"(var8) : "r"(var3), "r"(var4));
                asm volatile ("sub %0, %1, %2" : "=r"(var9) : "r"(var5), "r"(var6));
                
                // Mixed type inline asm - potential for secondary reloads
                int temp_int;
                double temp_double;
                asm volatile ("fcvt %0, %1" : "=r"(temp_int) : "w"(fvar0)); // FP to int
                asm volatile ("fcvt %0, %1" : "=w"(temp_double) : "r"(var7)); // int to FP
                
                // More register pressure with chained computations
                cvar0 = (char)(var8 & 0xFF);
                cvar1 = (char)(var9 & 0xFF);
                cvar2 = cvar0 + cvar1;
                cvar3 = cvar2 * 2;
                
                // Complex expression with many live variables
                fvar2 = fvar0 + fvar1 + (float)cvar3 + (float)var8 * 0.5f;
                fvar3 = fvar2 * 2.0f - (float)var9;
                fvar4 = fvar3 / (fvar1 + 1.0f);
                
                dvar2 = dvar0 * dvar1 + (double)fvar4;
                dvar3 = dvar2 / (dvar0 + 0.5);
                dvar4 = dvar3 - dvar1 * 2.0;
                
                // Force output reloads with volatile and pointer dereference
                vsink = var8 + var9 + (int)fvar2 + (int)dvar2;
                dsink = dvar4;
                
                // Assignment to array element with computed index - forces out reload
                int store_idx = (i * 11 + j * 13) % SIZE;
                matrix[store_idx][store_idx % SIZE] = var8 + var9;
                
                // More inline asm with memory constraints
                int mem_val;
                asm volatile ("ldr %0, [%1]" : "=r"(mem_val) : "r"(&matrix[i][j]));
                asm volatile ("str %0, [%1]" : : "r"(mem_val + 1), "r"(&matrix[j][i]));
                
                // Call helper function with many register arguments
                int func_result = helper_function(var0, var1, var2, var3, var4, fvar0, dvar0);
                
                // Use builtin that may require specific registers
                int popcnt_result = __builtin_popcount(func_result);
                
                // Chain everything together
                checksum += var0 + var1 + var2 + var3 + var4 + var5 + var6 + var7 +
                           (int)fvar0 + (int)fvar1 + (int)fvar2 + (int)fvar3 + (int)fvar4 +
                           (int)dvar0 + (int)dvar1 + (int)dvar2 + (int)dvar3 + (int)dvar4 +
                           cvar0 + cvar1 + cvar2 + cvar3 + func_result + popcnt_result;
                
                // Rotate variables to extend live ranges
                var0 = var1;
                var1 = var2;
                var2 = var3;
                var3 = var4;
                var4 = var5;
                var5 = var6;
                var6 = var7;
                var7 = var8 + var9;
                
                fvar0 = fvar1;
                fvar1 = fvar2;
                fvar2 = fvar3;
                fvar3 = fvar4;
                
                dvar0 = dvar1;
                dvar1 = dvar2;
                dvar2 = dvar3;
                dvar3 = dvar4;
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

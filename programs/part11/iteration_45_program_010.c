#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define NUM_VARS 25

typedef struct {
    int data[16];
    double values[8];
    char *ptr;
} ComplexStruct;

__attribute__((noinline))
int helper_function(int a, int b, int c, int d, int e, 
                    float f, double g, char h) {
    volatile int result = 0;
    // Force register usage
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %1, %2" : "+r"(result) : "r"(c), "r"(d));
    return result + (int)f + (int)g + (int)h + e;
}

int main() {
    int matrix[SIZE][SIZE];
    double big_array[SIZE * 2][SIZE / 2];
    ComplexStruct structs[SIZE];
    volatile int sink = 0;
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
        for (int j = 0; j < SIZE / 2; j++) {
            big_array[i][j] = i * 0.5 + j * 1.5;
        }
        for (int j = 0; j < 16; j++) {
            structs[i].data[j] = i * 16 + j;
        }
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, big_array, structs) map(from: checksum)
    {
        // Create massive register pressure with many local variables
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        float fvar1, fvar2, fvar3, fvar4, fvar5;
        double dvar1, dvar2, dvar3, dvar4;
        char cvar1, cvar2, cvar3, cvar4, cvar5, cvar6;
        volatile int vsink1, vsink2, vsink3;
        
        // Initialize from mapped arrays with complex addressing
        var1 = matrix[0][0];
        var2 = matrix[SIZE-1][SIZE-1];
        var3 = matrix[var1 % SIZE][var2 % SIZE];
        
        // Complex array indexing forcing address reloads
        int stride = SIZE / 4;
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                // Multi-dimensional array with complex index
                int idx = (i * stride + j * 3 + var3) % SIZE;
                int idx2 = ((i << 2) + (j >> 1) + var1) % SIZE;
                
                var4 = matrix[idx][idx2];
                var5 = matrix[idx2][idx];
                
                // Pointer chain access forcing base address computation
                int *ptr1 = structs[i % SIZE].data;
                int *ptr2 = structs[j % SIZE].data;
                
                // Complex addressing mode
                var6 = ptr1[(i * j) % 16];
                var7 = ptr2[(i + j * 2) % 16];
                
                // Mixed type operations forcing mode conversions
                fvar1 = (float)var4 * 1.5f;
                fvar2 = (float)var5 * 2.5f;
                dvar1 = (double)var6 * 3.14159;
                dvar2 = (double)var7 * 2.71828;
                
                // Inline assembly with register constraints
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %3, %4, %5"
                    : "=r"(var8), "+r"(var9)
                    : "r"(var4), "r"(var5), "r"(var6), "r"(var7)
                    : "cc"
                );
                
                // More assembly with mixed constraints
                int temp1, temp2;
                asm volatile (
                    "mov %0, %1\n\t"
                    "mov %2, %3"
                    : "=r"(temp1), "=m"(vsink1)
                    : "r"(var8), "m"(var9)
                );
                
                // Force output reload with volatile store
                vsink2 = var8 + var9;
                
                // Complex expression chain keeping many vars live
                var10 = var1 + var2 - var3 * var4 / (var5 + 1);
                var10 += var6 << 2;
                var10 |= var7 & 0xFF;
                
                // Floating point operations that might need FP registers
                fvar3 = fvar1 + fvar2 * fvar1 - fvar2;
                dvar3 = dvar1 * dvar2 + dvar1 / (dvar2 + 1.0);
                
                // Character operations with sign extension
                cvar1 = (char)(var10 & 0xFF);
                cvar2 = (char)((var10 >> 8) & 0xFF);
                cvar3 = cvar1 + cvar2;
                cvar4 = cvar1 - cvar2;
                cvar5 = cvar1 * cvar2;
                cvar6 = cvar1 | cvar2;
                
                // More inline assembly with specific constraints
                // This might trigger secondary reloads
                float fresult;
                asm volatile (
                    "fcvt %s0, %w1\n\t"
                    "fadd %s0, %s0, %s2"
                    : "=w"(fresult)
                    : "r"(var10), "w"(fvar3)
                );
                
                // Assign to array element with computed index - forces out reload
                int arr_idx = (i * 17 + j * 13) % SIZE;
                int arr_idx2 = (i * 11 + j * 19) % (SIZE / 2);
                big_array[arr_idx][arr_idx2] = dvar3 + fresult;
                
                // Struct member assignment with pointer arithmetic
                structs[(i + j) % SIZE].data[(i * j) % 16] = var10;
                
                // Call helper function with many register arguments
                int helper_result = helper_function(
                    var1, var2, var3, var4, var5,
                    fvar1, dvar1, cvar1
                );
                
                // Use all variables in final computation to keep them live
                checksum += var1 ^ var2 ^ var3 ^ var4 ^ var5 ^ var6 ^ var7;
                checksum += (int)fvar1 ^ (int)fvar2 ^ (int)fvar3;
                checksum += (int)dvar1 ^ (int)dvar2 ^ (int)dvar3;
                checksum += cvar1 + cvar2 + cvar3 + cvar4 + cvar5 + cvar6;
                checksum += helper_result;
                
                // Volatile sink to prevent optimization
                vsink3 = checksum;
            }
        }
        
        // Additional complex addressing
        for (int i = 0; i < 16; i++) {
            // Nested array access with pointer chain
            double *dptr = &big_array[i * 3 % SIZE][i * 5 % (SIZE / 2)];
            ComplexStruct *sptr = &structs[i * 7 % SIZE];
            
            // Force address computation in register
            *dptr = *dptr + sptr->data[i % 16] * 0.01;
            
            // More inline assembly
            double dtemp;
            asm volatile (
                "ldr %0, [%1]\n\t"
                "fadd %d0, %d0, %d2"
                : "=w"(dtemp)
                : "r"(dptr), "w"(dvar4)
            );
            
            checksum += (int)dtemp;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

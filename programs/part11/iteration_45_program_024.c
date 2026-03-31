#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define N_VARS 25

typedef struct {
    int data[16];
    double values[8];
    char *ptr;
} ComplexStruct;

__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = 0;
    // Force register usage with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("sub %0, %1, %2" : "+r"(result) : "r"(c), "r"(d));
    asm volatile ("mul %0, %1, %2" : "+r"(result) : "r"(e), "r"(f));
    asm volatile ("and %0, %1, %2" : "+r"(result) : "r"(g), "r"(h));
    return result;
}

int main() {
    // Initialize complex data structures
    int matrix1[SIZE][SIZE];
    double matrix2[SIZE][SIZE];
    ComplexStruct structs[SIZE];
    char buffer[SIZE * SIZE];
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * SIZE + j;
            matrix2[i][j] = (i * SIZE + j) * 0.1;
        }
        for (int k = 0; k < 16; k++) {
            structs[i].data[k] = i * 16 + k;
        }
        for (int k = 0; k < 8; k++) {
            structs[i].values[k] = (i * 8 + k) * 0.01;
        }
        structs[i].ptr = &buffer[i * SIZE];
    }
    
    int result = 0;
    
    #pragma omp target map(to: matrix1, matrix2, structs, buffer) \
                      map(tofrom: result) \
                      map(alloc: structs[0].ptr)
    {
        // Declare many local variables to create register pressure
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        float fvar1, fvar2, fvar3, fvar4, fvar5;
        double dvar1, dvar2, dvar3, dvar4, dvar5;
        char cvar1, cvar2, cvar3, cvar4, cvar5;
        volatile int vsink1, vsink2, vsink3;
        volatile double dsink1, dsink2;
        
        // Initialize from mapped arrays with complex addressing
        var1 = matrix1[0][0];
        var2 = matrix1[SIZE/2][SIZE/2];
        var3 = matrix1[SIZE-1][SIZE-1];
        
        // Complex addressing modes
        for (int i = 1; i < SIZE-1; i++) {
            for (int j = 1; j < SIZE-1; j++) {
                // Compute complex indices
                int idx1 = (i * SIZE + j) % SIZE;
                int idx2 = (i * 3 + j * 7) % SIZE;
                int idx3 = (i * 11 + j * 13) % SIZE;
                
                // Load many variables to keep them live
                var4 = matrix1[idx1][idx2];
                var5 = matrix1[idx2][idx3];
                var6 = matrix1[idx3][idx1];
                
                // Mixed type operations forcing conversions
                fvar1 = matrix2[i][j];
                dvar1 = matrix2[j][i];
                cvar1 = (char)(matrix1[i][j] & 0xFF);
                
                // Chain computations creating long live ranges
                var7 = var1 + var2 * var3 - var4 / (var5 + 1);
                var8 = var6 * var7 + (var2 << 2) - (var3 >> 1);
                var9 = (var4 & 0xF0F0) | (var5 & 0x0F0F);
                var10 = var8 ^ var9 + (var6 % 17);
                
                // Floating point chains
                fvar2 = fvar1 * 2.5f + (float)var7 * 0.1f;
                fvar3 = fvar2 / 3.14f - (float)var8 * 0.01f;
                dvar2 = dvar1 * 3.14159 + (double)fvar3;
                dvar3 = dvar2 / 2.71828 - (double)var9 * 0.001;
                
                // Character operations
                cvar2 = (char)(var10 & 0xFF);
                cvar3 = cvar1 + cvar2;
                cvar4 = cvar3 * 2 - cvar1;
                cvar5 = (cvar4 << 2) | (cvar3 >> 2);
                
                // Inline assembly with register constraints
                // Force specific register usage
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %0, %0, %3"
                    : "=r"(var7)
                    : "r"(var4), "r"(var5), "r"(var6)
                    : "cc"
                );
                
                // More assembly with mixed constraints
                int temp1, temp2;
                asm volatile (
                    "mov %0, %1\n\t"
                    "add %0, %0, %2"
                    : "=r"(temp1), "=r"(temp2)
                    : "r"(var8), "r"(var9), "1"(var10)
                );
                
                // Force output reloads with volatile assignments
                vsink1 = var7 + temp1;
                vsink2 = var8 * temp2;
                dsink1 = dvar2 + dvar3;
                
                // Complex struct access with pointer chain
                ComplexStruct *sptr = &structs[i];
                int *data_ptr = sptr->data;
                double *val_ptr = sptr->values;
                
                // Force address computation in registers
                int offset = (i * j) % 16;
                var1 = data_ptr[offset];
                var2 = data_ptr[(offset + 5) % 16];
                
                // Mixed register class usage (forcing secondary reloads)
                // Simulate moving between register classes
                #ifdef __aarch64__
                // AArch64 specific: move between general and FP registers
                double fp_temp;
                asm volatile (
                    "fmov %d0, %w1"
                    : "=w"(fp_temp)
                    : "r"(var3)
                );
                dsink2 = fp_temp;
                #else
                // x86 specific: use xmm registers
                double fp_temp;
                asm volatile (
                    "movd %0, %1"
                    : "=x"(fp_temp)
                    : "r"(var3)
                );
                dsink2 = fp_temp;
                #endif
                
                // Array assignment with computed index (forcing out reload)
                int *arr_ptr = matrix1[i];
                int comp_idx = (j * 7 + i * 3) % SIZE;
                arr_ptr[comp_idx] = var10 + temp1;
                
                // Pointer dereference assignment
                int *dyn_ptr = &matrix1[idx1][idx2];
                *dyn_ptr = var7 * var8 - var9;
                
                // Call helper function with many register arguments
                // Forces calling convention handling
                int call_result = helper_func(var1, var2, var3, var4, 
                                             var5, var6, var7, var8);
                
                // Use result in complex expression
                var10 = call_result + (var9 << 3) - (var8 >> 2);
                
                // More floating point with type mixing
                fvar4 = (float)var10 * 0.5f + fvar3;
                fvar5 = fvar4 - (float)call_result * 0.25f;
                dvar4 = (double)fvar5 + dvar3;
                dvar5 = dvar4 * 2.0 - (double)var7;
                
                // Final volatile store
                vsink3 = var10 + call_result;
                
                // Update result checksum
                result += var7 + var8 + var9 + var10 + (int)fvar5 + (int)dvar5;
            }
        }
        
        // Additional register pressure with unrolled loop
        for (int k = 0; k < 8; k++) {
            // Use builtins that may require specific registers
            int popcnt = __builtin_popcount(var1 + k);
            int clz = __builtin_clz(var2 + k);
            int ctz = __builtin_ctz(var3 + k);
            
            // Chain these results
            var4 = popcnt + clz - ctz;
            var5 = (var4 << k) | (var3 >> k);
            
            // More inline asm with memory constraints
            asm volatile (
                "ldr %0, [%1]\n\t"
                "str %2, [%1, #4]"
                : "=r"(var6)
                : "r"(&matrix1[k][0]), "r"(var5)
                : "memory"
            );
            
            result += var4 + var5 + var6;
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}

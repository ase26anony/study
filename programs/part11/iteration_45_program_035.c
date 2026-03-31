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
int helper_func(int a, int b, float c, double d, char e, int f, float g, double h) {
    volatile int result;
    result = (int)(a * b + c * d + e + f + g + h);
    return result;
}

int main() {
    int matrix[SIZE][SIZE];
    DataStruct ds1, ds2;
    int *dynamic_arr = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
        ds1.data[i] = i * 2;
        ds1.values[i] = i * 3.14;
        ds2.data[i] = i * 3;
        ds2.values[i] = i * 2.71;
    }
    
    ds1.ptr = (char*)dynamic_arr;
    ds2.ptr = ds1.ptr + SIZE * sizeof(int);
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, ds1, ds2) \
                      map(tofrom: dynamic_arr[0:SIZE*SIZE]) \
                      map(from: checksum)
    {
        // Declare many local variables to create register pressure
        int var0, var1, var2, var3, var4, var5, var6, var7, var8, var9;
        float fvar0, fvar1, fvar2, fvar3, fvar4;
        double dvar0, dvar1, dvar2, dvar3;
        char cvar0, cvar1, cvar2;
        volatile int vsink;
        volatile double dsink;
        
        // Initialize from mapped data with complex addressing
        var0 = matrix[0][0];
        var1 = ds1.data[var0 % SIZE];
        var2 = ds2.data[(var0 * 3) % SIZE];
        fvar0 = (float)ds1.values[var1 % SIZE];
        dvar0 = ds2.values[var2 % SIZE];
        cvar0 = (char)(var0 + var1 + var2);
        
        // Complex nested loops with register-intensive computations
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                // Complex array indexing forcing address reloads
                int idx = (i * 37 + j * 13) % SIZE;
                int idx2 = (i * 19 + j * 7) % SIZE;
                
                // Chain many variables together
                var3 = matrix[idx][idx2];
                var4 = ds1.data[(idx * 2 + idx2) % SIZE];
                var5 = ds2.data[(idx + idx2 * 3) % SIZE];
                
                // Mixed-type computations
                fvar1 = fvar0 + (float)var3 * 0.5f;
                fvar2 = (float)var4 * 1.5f - fvar1;
                dvar1 = dvar0 + (double)var5 * 0.25;
                dvar2 = dvar1 * 2.0 - (double)fvar2;
                
                // Inline assembly with register constraints
                // Force register conflicts
                asm volatile (
                    "add %0, %1, %2\n\t"
                    : "=r"(var6)
                    : "r"(var3), "r"(var4)
                );
                
                asm volatile (
                    "mul %0, %1, %2\n\t"
                    : "=r"(var7)
                    : "r"(var5), "r"(var6)
                );
                
                // More variables to increase pressure
                var8 = var6 + var7;
                var9 = var8 * 2 - var5;
                
                // Floating-point inline assembly (may need secondary reloads)
                #ifdef __aarch64__
                asm volatile (
                    "fmov %s0, %w1\n\t"
                    : "=w"(fvar3)
                    : "r"(var8)
                );
                #else
                // x86 version
                asm volatile (
                    "movd %1, %0\n\t"
                    : "=x"(fvar3)
                    : "r"(var8)
                );
                #endif
                
                fvar4 = fvar3 + fvar2;
                
                // Complex pointer arithmetic forcing address computation
                char *ptr1 = ds1.ptr + idx * sizeof(int);
                char *ptr2 = ds2.ptr + idx2 * sizeof(int);
                
                // Assignments to volatile (forcing out reloads)
                vsink = var9;
                dsink = dvar2;
                
                // Assignment through pointer with computed address
                *(int*)ptr1 = var8;
                *(int*)ptr2 = var9;
                
                // Multi-dimensional array with complex index
                int arr_idx = (i * 32 + j) % (SIZE * SIZE);
                dynamic_arr[arr_idx] = var7 + var8;
                
                // Mixed mode operations
                cvar1 = (char)(var3 & 0xFF);
                cvar2 = cvar0 + cvar1;
                var0 = var0 + (int)cvar2;
                
                // Function call with many register arguments
                int call_result = helper_func(var0, var1, fvar0, dvar0, 
                                             cvar0, var2, fvar1, dvar1);
                
                // More chaining
                var1 = var1 + call_result;
                var2 = var2 * 2 - call_result;
                fvar0 = fvar0 + (float)call_result * 0.1f;
                dvar0 = dvar0 + (double)call_result * 0.01;
                
                // Update checksum
                checksum += var0 + var1 + var2 + (int)fvar0 + (int)dvar0;
            }
        }
        
        // Additional register pressure block
        for (int k = 0; k < 16; k++) {
            // Use builtins that may require specific registers
            int popcnt_var = __builtin_popcount(k);
            int clz_var = __builtin_clz(k | 1);
            
            // More inline assembly with constraints
            int temp1, temp2;
            asm volatile (
                "and %0, %1, %2\n\t"
                "orr %3, %0, %1\n\t"
                : "=r"(temp1), "=r"(temp2)
                : "r"(popcnt_var), "r"(clz_var), "1"(var0)
            );
            
            // Complex struct member access
            int struct_idx = (k * 7) % SIZE;
            double complex_val = ds1.values[struct_idx] * 
                                ds2.values[(struct_idx * 3) % SIZE] +
                                matrix[struct_idx % SIZE][k % SIZE];
            
            // Force store with complex address
            volatile double *volatile_ptr = &dsink;
            *volatile_ptr = complex_val;
            
            checksum += temp1 + temp2 + (int)complex_val;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    free(dynamic_arr);
    
    return 0;
}

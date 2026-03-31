#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 128
#define NUM_VARS 25

struct DataStruct {
    int data[SIZE];
    float fdata[SIZE];
    double ddata[SIZE];
    char* next;
};

// Force register usage with noinline attribute
__attribute__((noinline)) 
int helper_func(int a, int b, int c, float d, double e, char f, int g) {
    volatile int result;
    // Force register pressure with inline asm
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c)
        : "cc"
    );
    return result + (int)d + (int)e + f + g;
}

int main() {
    // Initialize complex data structures
    struct DataStruct ds;
    int matrix[SIZE][SIZE];
    double dmatrix[SIZE][SIZE];
    volatile int sink; // For forcing output reloads
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        ds.data[i] = i;
        ds.fdata[i] = i * 1.5f;
        ds.ddata[i] = i * 2.5;
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
            dmatrix[i][j] = i * SIZE + j * 0.5;
        }
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: ds, matrix, dmatrix) map(from: checksum)
    {
        // Declare many local variables to create register pressure
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        float fvar1, fvar2, fvar3, fvar4, fvar5;
        double dvar1, dvar2, dvar3, dvar4, dvar5;
        char cvar1, cvar2, cvar3, cvar4, cvar5;
        register int reg_var1, reg_var2, reg_var3; // Hint for register allocation
        
        // Initialize from mapped data with complex addressing
        var1 = ds.data[0];
        var2 = matrix[0][0];
        var3 = ds.data[1] + matrix[1][1];
        fvar1 = ds.fdata[0];
        dvar1 = ds.ddata[0];
        cvar1 = (char)ds.data[0];
        
        // Complex nested loop with register pressure
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                // Complex array indexing forcing address reloads
                int idx = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 19) % SIZE;
                
                // Chain many computations to keep variables live
                var4 = matrix[idx][idx2] + var1;
                var5 = ds.data[idx] * var2 - var3;
                
                // Mixed type operations forcing mode conversions
                fvar2 = fvar1 + (float)var4 * 0.5f;
                dvar2 = dvar1 + (double)var5 * 0.25;
                
                // Inline assembly with register constraints
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %0, %0, %3"
                    : "=r"(var6)
                    : "r"(var4), "r"(var5), "r"(idx)
                    : "cc"
                );
                
                // More complex chaining
                var7 = var6 * 3 + idx;
                var8 = var7 - idx2;
                
                // Force output reload with volatile and pointer dereference
                int* ptr = &var8;
                sink = *ptr;  // Forces store through register
                
                // Another inline asm with different constraints
                asm volatile (
                    "mul %0, %1, %2"
                    : "=r"(var9)
                    : "r"(var7), "r"(var8)
                    : "cc"
                );
                
                // Mixed type computation
                fvar3 = fvar2 + (float)var9 / 10.0f;
                dvar3 = dvar2 + (double)var9 / 20.0;
                
                // Complex addressing with struct member
                int complex_idx = (i * SIZE + j) % SIZE;
                var10 = ds.data[complex_idx] + matrix[i % SIZE][j % SIZE];
                
                // Force secondary reload scenario with mixed register classes
                float temp_float;
                asm volatile (
                    "fcvt %s0, %w1\n\t"  // Convert integer to float (AArch64 style)
                    "fadd %s0, %s0, %s2"
                    : "=w"(temp_float)
                    : "r"(var10), "w"(fvar3)
                    : 
                );
                fvar4 = temp_float;
                
                // More register pressure
                cvar2 = (char)(var9 % 256);
                cvar3 = cvar1 + cvar2;
                cvar4 = cvar3 * 2;
                
                // Use register variables
                reg_var1 = var9 + var10;
                reg_var2 = reg_var1 * 2;
                reg_var3 = reg_var2 - var8;
                
                // Complex expression with many live variables
                dvar4 = dvar3 + (double)reg_var3 * 0.01 + 
                       (double)fvar4 * 0.02 + (double)cvar4;
                
                // Force another output reload with array element
                int arr_idx = (i * 11 + j * 7) % 16;
                int temp_arr[16];
                temp_arr[arr_idx] = reg_var3;  // Computed index forces address reload
                
                // Call helper function with many arguments
                int helper_result = helper_func(
                    var9, var10, reg_var1, 
                    fvar4, dvar4, cvar4, arr_idx
                );
                
                // Final computation for checksum
                checksum += helper_result + temp_arr[arr_idx];
                
                // Rotate variables to keep them live
                var1 = var2;
                var2 = var3;
                var3 = var4;
                fvar1 = fvar2;
                dvar1 = dvar2;
                cvar1 = cvar2;
            }
        }
        
        // Additional complex operations outside loops
        for (int k = 0; k < 8; k++) {
            // Pointer arithmetic forcing address computation
            int* base_ptr = &ds.data[0];
            int offset = (k * 29) % SIZE;
            int* elem_ptr = base_ptr + offset;
            
            // Dereference with computed pointer
            int val = *elem_ptr;
            
            // More inline asm with memory constraint
            asm volatile (
                "ldr %0, [%1]\n\t"
                "add %0, %0, #1"
                : "=r"(val)
                : "r"(elem_ptr)
                : "memory"
            );
            
            // Assign to volatile through pointer
            int* volatile_ptr = (int*)&sink;
            *volatile_ptr = val;  // Forces output reload
            
            checksum += val;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

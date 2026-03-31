#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define N_VARS 25

typedef struct {
    int data[SIZE];
    double values[SIZE];
    char *ptr;
} DataStruct;

__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result;
    // Force register usage and potential reloads
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %1, %2" : "+r"(result) : "r"(c), "r"(d));
    return result + e + f + g + h;
}

int main() {
    int matrix[SIZE][SIZE];
    double arr3d[SIZE][SIZE][SIZE];
    DataStruct ds1, ds2;
    int *ptr_array[SIZE];
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
        ptr_array[i] = &matrix[i][0];
        ds1.data[i] = i * 2;
        ds1.values[i] = i * 3.14;
    }
    ds1.ptr = (char*)matrix;
    ds2 = ds1;
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, arr3d, ds1, ds2, ptr_array) \
                      map(from: checksum)
    {
        // Declare many local variables to create register pressure
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        int var11, var12, var13, var14, var15, var16, var17, var18, var19, var20;
        float fvar1, fvar2, fvar3, fvar4, fvar5;
        double dvar1, dvar2, dvar3, dvar4;
        char cvar1, cvar2, cvar3;
        volatile int vsink1, vsink2, vsink3;
        volatile double dsink1;
        
        // Initialize from mapped data with complex addressing
        var1 = matrix[0][0];
        var2 = matrix[SIZE-1][SIZE-1];
        var3 = ds1.data[var1 % SIZE];
        var4 = ds2.data[var2 % SIZE];
        
        // Complex pointer chain access
        int *ptr1 = ptr_array[var1 % SIZE];
        int *ptr2 = ptr_array[var2 % SIZE];
        
        // Nested loops with complex array indexing
        for (int i = 1; i < 8; i++) {
            for (int j = 1; j < 8; j++) {
                // Complex index calculation forcing address reloads
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 19) % SIZE;
                int idx3 = (i * 31 + j * 29) % SIZE;
                
                // Multi-dimensional array access with complex indices
                var5 = matrix[idx1][idx2];
                var6 = matrix[idx2][idx3];
                
                // Struct member access with pointer arithmetic
                var7 = ds1.data[(idx1 + idx2) % SIZE];
                var8 = ds2.data[(idx2 + idx3) % SIZE];
                
                // Mixed type operations forcing mode conversions
                fvar1 = (float)var5 * 1.5f;
                fvar2 = (float)var6 * 2.5f;
                dvar1 = (double)var7 * 3.14159;
                dvar2 = (double)var8 * 2.71828;
                
                // Inline assembly with register constraints
                // Force specific register usage
                asm volatile ("add %0, %1, %2" : "=r"(var9) : "r"(var5), "r"(var6));
                asm volatile ("sub %0, %1, %2" : "=r"(var10) : "r"(var7), "r"(var8));
                
                // Memory constraint forcing address computation
                asm volatile ("ldr %0, [%1]" : "=r"(var11) : "r"(ptr1 + idx1));
                asm volatile ("ldr %0, [%1]" : "=r"(var12) : "r"(ptr2 + idx2));
                
                // Chain computations to keep variables live
                var13 = var9 + var10;
                var14 = var11 * var12;
                var15 = var13 - var14;
                var16 = var15 + (int)fvar1;
                var17 = var16 - (int)fvar2;
                var18 = var17 + (int)dvar1;
                var19 = var18 - (int)dvar2;
                
                // Character operations
                cvar1 = (char)(var19 & 0xFF);
                cvar2 = (char)((var19 >> 8) & 0xFF);
                cvar3 = cvar1 + cvar2;
                var20 = var19 + cvar3;
                
                // Force output reloads with volatile assignments
                vsink1 = var20;
                vsink2 = var19;
                dsink1 = dvar1 + dvar2;
                
                // Assignment to computed array index (forcing address reload)
                int * volatile out_ptr = ptr_array[(i + j) % SIZE];
                out_ptr[idx1 % 16] = var20;
                
                // More complex addressing with struct pointer
                char *char_ptr = ds1.ptr + idx1 * sizeof(int);
                asm volatile ("str %1, [%0]" : : "r"(char_ptr), "r"(var20));
                
                // Call helper function with many register arguments
                // Forces calling convention handling and potential reloads
                int func_result = helper_func(var1, var2, var3, var4, var5, 
                                             var6, var7, var8);
                
                // Secondary reload scenario: mixed register classes
                // Simulate move between different register classes
                int ival = func_result;
                double dval;
                // This might require secondary reload on some architectures
                asm volatile ("ucvtf %d0, %w1" : "=w"(dval) : "r"(ival));
                
                // Use builtin that may require specific registers
                int popcnt = __builtin_popcount(ival);
                
                // More arithmetic chaining
                fvar3 = fvar1 + fvar2 + (float)dval;
                fvar4 = fvar3 * 2.0f;
                fvar5 = fvar4 / 3.0f;
                
                dvar3 = dvar1 + dvar2 + (double)fvar5;
                dvar4 = dvar3 * 1.5;
                
                // Final volatile store
                vsink3 = popcnt + (int)dvar4;
                
                // Update checksum
                checksum += var20 + func_result + popcnt;
            }
        }
        
        // Additional register pressure with many simultaneous live variables
        int final1 = var1 + var2 + var3 + var4;
        int final2 = var5 + var6 + var7 + var8;
        int final3 = var9 + var10 + var11 + var12;
        int final4 = var13 + var14 + var15 + var16;
        int final5 = var17 + var18 + var19 + var20;
        
        float ffinal = fvar1 + fvar2 + fvar3 + fvar4 + fvar5;
        double dfinal = dvar1 + dvar2 + dvar3 + dvar4;
        
        // Force one more complex output reload
        volatile int *final_out = (volatile int*)ds1.ptr;
        final_out[0] = final1 + final2 + final3 + final4 + final5 + 
                      (int)ffinal + (int)dfinal;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

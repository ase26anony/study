#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define NUM_VARS 25

typedef struct {
    int data[SIZE];
    double values[SIZE];
    char *ptr;
    volatile int flag;
} ComplexStruct;

__attribute__((noinline))
int helper_func(int a, int b, int c, double d, float e, char f) {
    volatile int result = 0;
    // Force register usage with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("add %0, %0, %1" : "+r"(result) : "r"(c));
    return result + (int)(d * e) + f;
}

int main() {
    // Initialize complex data structures
    ComplexStruct cs;
    int matrix[SIZE][SIZE];
    double arr3d[SIZE][SIZE][SIZE];
    volatile int sink;
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        cs.data[i] = i;
        cs.values[i] = i * 1.5;
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
            for (int k = 0; k < SIZE; k++) {
                arr3d[i][j][k] = i * j * k * 0.1;
            }
        }
    }
    
    cs.ptr = (char*)malloc(SIZE * sizeof(char));
    for (int i = 0; i < SIZE; i++) {
        cs.ptr[i] = i % 128;
    }
    
    int result = 0;
    
    #pragma omp target map(to: cs, matrix, arr3d) map(tofrom: result) \
                      map(tofrom: cs.ptr[0:SIZE])
    {
        // Declare many local variables to create register pressure
        int var1 = cs.data[0];
        int var2 = matrix[0][0];
        double var3 = cs.values[0];
        float var4 = var3 * 0.5f;
        char var5 = cs.ptr[0];
        int var6 = var1 + var2;
        double var7 = var3 * 2.0;
        float var8 = var4 / 2.0f;
        char var9 = var5 + 1;
        int var10 = var6 * 2;
        double var11 = var7 + 1.0;
        float var12 = var8 * 3.0f;
        int var13 = var10 - var1;
        double var14 = var11 / var3;
        float var15 = var12 + var4;
        char var16 = var9 * 2;
        int var17 = var13 + var2;
        double var18 = var14 * var7;
        float var19 = var15 - var8;
        int var20 = var17 % 256;
        double var21 = var18 + var11;
        float var22 = var19 * 2.0f;
        char var23 = var16 + var5;
        int var24 = var20 ^ var1;
        double var25 = var21 / var14;
        
        // Complex nested loops with addressing that requires reloads
        for (int i = 1; i < 16; i++) {
            for (int j = 1; j < 16; j++) {
                // Complex array indexing requiring address computation
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 19) % SIZE;
                int idx3 = (i * 29 + j * 31) % SIZE;
                
                // Force output reload with pointer dereference
                int *ptr1 = &cs.data[idx1];
                double *ptr2 = &cs.values[idx2];
                
                // Complex addressing with multiple computations
                int matrix_idx = (i * SIZE + j) % SIZE;
                double arr3d_val = arr3d[idx1][idx2][idx3];
                
                // Chain computations keeping many variables live
                var1 = matrix[i][j] + var1;
                var2 = cs.data[idx1] * var2;
                var3 = arr3d_val + var3;
                var4 = (float)var3 * var4;
                var5 = cs.ptr[idx2] + var5;
                
                // Inline assembly with register constraints
                asm volatile ("add %0, %1, %2" : "=r"(var6) : "r"(var1), "r"(var2));
                asm volatile ("mul %0, %1, %2" : "=r"(var7) : "r"(var3), "r"(var6));
                
                // Mixed type operations forcing mode conversions
                var8 = (float)var7 + var4;
                var9 = (char)(var5 + var6 % 256);
                
                // Force address reload for store operation
                *ptr1 = var1 + var2;
                *ptr2 = var3 + var7;
                
                // Volatile assignment forcing output reload
                sink = var6;
                
                // Complex expression with struct member access
                int complex_expr = cs.data[(i * j + idx1) % SIZE] + 
                                   matrix[matrix_idx][idx2] * 
                                   (int)arr3d[idx3][idx1][idx2];
                
                // More inline assembly with specific constraints
                int temp1, temp2;
                asm volatile ("mov %0, %1" : "=r"(temp1) : "r"(complex_expr));
                asm volatile ("and %0, %1, %2" : "=r"(temp2) : "r"(temp1), "r"(var24));
                
                // Force secondary reload scenario
                #ifdef __aarch64__
                // Move between general and FP registers (may need secondary reload)
                double fp_val;
                asm volatile ("fmov %d0, %1" : "=w"(fp_val) : "r"(temp2));
                var25 = fp_val + var25;
                #else
                // x86 version with xmm registers
                double fp_val;
                asm volatile ("movq %1, %0" : "=x"(fp_val) : "r"(temp2));
                var25 = fp_val + var25;
                #endif
                
                // Chain more variables
                var10 = var6 + var10;
                var11 = var7 * var11;
                var12 = var8 + var12;
                var13 = var9 * var13;
                var14 = var10 + var14;
                var15 = var11 * var15;
                var16 = var12 + var16;
                var17 = var13 * var17;
                var18 = var14 + var18;
                var19 = var15 * var19;
                var20 = var16 + var20;
                var21 = var17 * var21;
                var22 = var18 + var22;
                var23 = var19 * var23;
                var24 = var20 + var24;
            }
            
            // Function call with multiple register arguments
            int func_result = helper_func(var1, var2, var3, var4, var5, var6);
            result += func_result;
            
            // More complex addressing
            int offset = i * 7;
            char *char_ptr = cs.ptr + offset;
            *char_ptr = (char)(var1 % 256);
            
            // Array assignment with computed index
            matrix[(i * 11) % SIZE][(i * 13) % SIZE] = var2 + result;
        }
        
        // Final computation using all variables
        result += var1 + var2 + (int)var3 + (int)var4 + var5 + var6 + 
                 (int)var7 + (int)var8 + var9 + var10 + (int)var11 + 
                 (int)var12 + var13 + (int)var14 + (int)var15 + var16 + 
                 var17 + (int)var18 + (int)var19 + var20 + (int)var21 + 
                 (int)var22 + var23 + var24 + (int)var25;
    }
    
    free(cs.ptr);
    printf("Result: %d\n", result);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define VARS 25

typedef struct {
    int data[SIZE];
    double values[SIZE];
    char *ptr;
} ComplexStruct;

__attribute__((noinline))
int helper_func(int a, int b, float c, double d, char e, int f, float g) {
    volatile int result;
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    return result + (int)c + (int)d + e + f + (int)g;
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
    
    int checksum = 0;
    
    #pragma omp target map(to: cs, matrix, arr3d) map(tofrom: checksum)
    {
        // Declare many local variables to create register pressure
        int var1 = cs.data[0];
        int var2 = cs.data[1];
        int var3 = cs.data[2];
        float var4 = cs.values[0];
        float var5 = cs.values[1];
        double var6 = cs.values[2];
        double var7 = cs.values[3];
        char var8 = cs.ptr[0];
        char var9 = cs.ptr[1];
        int var10 = matrix[0][0];
        int var11 = matrix[1][1];
        float var12 = arr3d[0][0][0];
        double var13 = arr3d[1][1][1];
        int var14 = cs.data[3];
        int var15 = cs.data[4];
        float var16 = cs.values[4];
        double var17 = cs.values[5];
        char var18 = cs.ptr[2];
        int var19 = matrix[2][2];
        int var20 = matrix[3][3];
        float var21 = arr3d[2][2][2];
        double var22 = arr3d[3][3][3];
        int var23 = 0;
        float var24 = 0.0f;
        double var25 = 0.0;
        
        // Complex nested loops with register pressure
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                // Complex addressing modes that require address reloads
                int idx1 = (i * 16 + j * 2) % SIZE;
                int idx2 = (i * 8 + j * 4 + var1) % SIZE;
                int idx3 = (i * 4 + j * 8 + var2) % SIZE;
                
                // Force output reloads with pointer dereferences
                int *ptr1 = &cs.data[idx1];
                double *ptr2 = &cs.values[idx2];
                char *ptr3 = &cs.ptr[idx3];
                
                // Complex expressions chaining many variables
                var1 = var1 + var2 - var3 * var10;
                var4 = var4 + var5 * var12;
                var6 = var6 + var7 / (var13 + 1.0);
                var8 = var8 + var9 - var18;
                
                // Inline assembly with register constraints
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %3, %4, %5"
                    : "=r"(var10), "=r"(var11), "+r"(var14), "=r"(var15)
                    : "r"(var1), "r"(var2), "0"(var10), "1"(var11), "2"(var14), "3"(var15)
                    : "cc"
                );
                
                // Mixed type operations forcing mode conversions
                var12 = var12 + var4 + var5;
                var13 = var13 + var6 + var7 + var21 + var22;
                
                // Force secondary reloads with specific register classes
                #ifdef __aarch64__
                asm volatile (
                    "fmov %s0, %w1\n\t"
                    "fmov %s2, %w3"
                    : "=w"(var24), "=w"(var21)
                    : "r"(var23), "r"(var19), "0"(var24), "1"(var21)
                );
                #elif __x86_64__
                asm volatile (
                    "movd %1, %0\n\t"
                    "movd %3, %2"
                    : "=x"(var24), "=x"(var21)
                    : "r"(var23), "r"(var19), "0"(var24), "1"(var21)
                );
                #endif
                
                // Assignments to volatile (forcing output reloads)
                sink = var1 + var2 + var3 + var10 + var11;
                
                // Complex array accesses with computed indices
                matrix[idx1][idx2] = var1 + var2 + var3;
                cs.data[(i * 32 + j * 16 + var15) % SIZE] = var10 * var11;
                
                // Pointer chain access
                *ptr1 = var1;
                *ptr2 = var6 + var7;
                *ptr3 = var8 + var9;
                
                // Call helper function with many register arguments
                var23 = helper_func(var1, var2, var4, var6, var8, var10, var12);
                
                // Chain computations keeping all variables live
                var25 = var13 + var6 + var7 + var21 + var22 + var24;
                var19 = var19 + var20 + var23;
                var20 = var20 - var19 + var23;
                
                // More inline assembly with memory constraints
                asm volatile (
                    "ldr %0, [%1]\n\t"
                    "str %2, [%3]"
                    : "=r"(var14)
                    : "r"(ptr1), "r"(var15), "r"(ptr1)
                    : "memory"
                );
            }
        }
        
        // Final checksum computation using all variables
        checksum = var1 + var2 + var3 + (int)var4 + (int)var5 + (int)var6 + 
                  (int)var7 + var8 + var9 + var10 + var11 + (int)var12 + 
                  (int)var13 + var14 + var15 + (int)var16 + (int)var17 + 
                  var18 + var19 + var20 + (int)var21 + (int)var22 + var23 + 
                  (int)var24 + (int)var25;
    }
    
    printf("Checksum: %d\n", checksum);
    free(cs.ptr);
    
    return 0;
}

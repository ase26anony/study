#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define NUM_VARS 20

typedef struct {
    int data[SIZE];
    double values[SIZE];
    char *ptr;
} ComplexStruct;

__attribute__((noinline))
int helper_function(int a, int b, int c, int d, int e, 
                    float f, double g, char h) {
    volatile int result;
    // Force register usage with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %0, %1" : "+r"(result) : "r"(c));
    return result + (int)f + (int)g + (int)h;
}

int main() {
    // Initialize complex data structures
    ComplexStruct cs;
    int matrix[SIZE][SIZE];
    double arr3d[SIZE][SIZE][SIZE];
    volatile int sink;  // For forcing output reloads
    
    // Initialize with some data
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
        cs.ptr[i] = i % 256;
    }
    
    int result = 0;
    
    #pragma omp target map(to: cs, matrix, arr3d) map(from: result) \
                      map(tofrom: cs.ptr[0:SIZE])
    {
        // Declare many local variables to create register pressure
        int var1 = cs.data[0];
        int var2 = cs.data[1];
        int var3 = cs.data[2];
        int var4 = cs.data[3];
        int var5 = cs.data[4];
        float var6 = cs.values[0];
        float var7 = cs.values[1];
        float var8 = cs.values[2];
        double var9 = cs.values[3];
        double var10 = cs.values[4];
        char var11 = cs.ptr[0];
        char var12 = cs.ptr[1];
        char var13 = cs.ptr[2];
        short var14 = cs.data[0] & 0xFFFF;
        short var15 = cs.data[1] & 0xFFFF;
        long var16 = cs.data[0] * 1000L;
        long var17 = cs.data[1] * 1000L;
        unsigned int var18 = cs.data[0];
        unsigned int var19 = cs.data[1];
        unsigned long var20 = cs.data[0] * 1000UL;
        
        // Complex nested loops with register pressure
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                // Complex addressing modes that may need reloads
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 19) % SIZE;
                int idx3 = (i * 29 + j * 31) % SIZE;
                
                // Chain computations to keep variables live
                var1 = matrix[idx1][idx2] + var2;
                var2 = matrix[idx2][idx3] * var3;
                var3 = var1 ^ var2;
                
                // Mixed type operations forcing mode conversions
                var6 = var1 * 0.5f + var6;
                var7 = var2 * 0.3f + var7;
                var9 = var3 * 0.1 + var9;
                var10 = var1 * 0.2 + var10;
                
                // Inline assembly with register constraints
                // Force specific register usage
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %3, %4, %5"
                    : "=r"(var4), "+r"(var5), "=r"(var11)
                    : "r"(var1), "r"(var2), "r"(var3),
                      "0"(var4), "1"(var5), "2"(var11)
                    : "cc"
                );
                
                // More inline assembly with mixed constraints
                // Force memory operands
                int temp;
                asm volatile (
                    "ldr %0, [%1, %2, lsl #2]\n\t"
                    "add %0, %0, #1"
                    : "=r"(temp)
                    : "r"(cs.data), "r"(idx1)
                    : "memory"
                );
                
                // Force output reloads with volatile and pointer derefs
                sink = var1 + var2 + var3;
                cs.data[idx1] = var4 + var5;
                cs.ptr[idx2] = var11 + var12;
                
                // Complex array access with multi-dimensional indexing
                double* ptr = &arr3d[idx1][idx2][idx3];
                *ptr = var9 + var10 + (idx1 * idx2 * 0.01);
                
                // Force secondary reloads by moving between register classes
                // This may require FP to integer moves or vice versa
                int int_from_fp;
                asm volatile (
                    "fcvtzs %w0, %s1"
                    : "=r"(int_from_fp)
                    : "w"(var6)
                );
                
                float fp_from_int;
                asm volatile (
                    "scvtf %s0, %w1"
                    : "=w"(fp_from_int)
                    : "r"(var4)
                );
                
                // Chain more operations
                var12 = (var11 + int_from_fp) % 256;
                var13 = (var12 + var4) % 256;
                var14 = var1 + var2;
                var15 = var3 + var4;
                var16 = var1 * var2 * 1000L;
                var17 = var3 * var4 * 1000L;
                var18 = var1 ^ var2 ^ var3;
                var19 = var4 | var5 | var6;
                var20 = var16 + var17 + var18;
                
                // Function call with many register arguments
                // Forces calling convention handling and potential reloads
                int func_result = helper_function(
                    var1, var2, var3, var4, var5,
                    var6, var9, var11
                );
                
                // Use the result
                var1 = func_result % 1000;
                
                // More complex addressing
                int complex_idx = (i * SIZE * 3 + j * SIZE * 7) % SIZE;
                int* complex_ptr = &matrix[complex_idx % SIZE][(complex_idx * 2) % SIZE];
                *complex_ptr = var1 + var2 + var3 + var4;
                
                // Volatile store forcing output reload
                volatile int* volatile_ptr = (volatile int*)&cs.data[idx3];
                *volatile_ptr = var5 + var6 + var7;
            }
        }
        
        // Final computation using all variables
        result = var1 + var2 + var3 + var4 + var5 + 
                 (int)var6 + (int)var7 + (int)var8 + 
                 (int)var9 + (int)var10 + var11 + var12 + 
                 var13 + var14 + var15 + (int)var16 + 
                 (int)var17 + var18 + var19 + (int)var20;
    }
    
    printf("Result: %d\n", result);
    free(cs.ptr);
    
    return 0;
}

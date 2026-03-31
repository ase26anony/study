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
int helper_func(int a, int b, int c, int d, int e, float f, double g) {
    volatile int result = 0;
    // Force register usage with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %1, %2" : "+r"(result) : "r"(c), "r"(d));
    return result + (int)(f * g) + e;
}

int main() {
    // Initialize complex data structures
    int matrix1[SIZE][SIZE];
    int matrix2[SIZE][SIZE];
    double dmatrix[SIZE][SIZE];
    ComplexStruct structs[SIZE];
    volatile int sink = 0;
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * SIZE + j;
            matrix2[i][j] = (i + j) % 256;
            dmatrix[i][j] = (i * 0.1) + (j * 0.01);
        }
        for (int k = 0; k < 16; k++) {
            structs[i].data[k] = i * 16 + k;
        }
        for (int k = 0; k < 8; k++) {
            structs[i].values[k] = i * 0.5 + k * 0.1;
        }
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix1, matrix2, dmatrix, structs) map(from: checksum)
    {
        // Declare many local variables to create register pressure
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        int var11, var12, var13, var14, var15, var16, var17, var18, var19, var20;
        float fvar1, fvar2, fvar3, fvar4, fvar5;
        double dvar1, dvar2, dvar3, dvar4;
        char cvar1, cvar2, cvar3, cvar4;
        volatile int vsink1, vsink2, vsink3;
        
        // Initialize from mapped arrays with complex indexing
        var1 = matrix1[0][0];
        var2 = matrix2[1][1];
        var3 = matrix1[2][2];
        var4 = matrix2[3][3];
        var5 = matrix1[4][4];
        
        fvar1 = dmatrix[0][0];
        fvar2 = dmatrix[1][1];
        dvar1 = dmatrix[2][2];
        dvar2 = dmatrix[3][3];
        
        // Complex pointer chain access
        int *ptr1 = &matrix1[10][10];
        int *ptr2 = &matrix2[20][20];
        double *dptr = &dmatrix[30][30];
        
        // Nested loops creating complex addressing
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                // Complex array indexing forcing address computation
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 19) % SIZE;
                int idx3 = (i * 29 + j * 31) % SIZE;
                
                // Chain many variables together
                var6 = matrix1[idx1][idx2] + matrix2[idx2][idx3];
                var7 = matrix2[idx3][idx1] * var1;
                var8 = var6 + var7 - var2;
                var9 = var8 * 3 + var3;
                var10 = var9 / 2 + var4;
                
                // Mixed type operations forcing mode conversions
                fvar3 = fvar1 + var5;  // int to float
                dvar3 = dvar1 + fvar2; // float to double
                cvar1 = var6 & 0xFF;   // int to char
                
                // Inline assembly with register constraints
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %0, %0, %3"
                    : "=r"(var11)
                    : "r"(var7), "r"(var8), "r"(var9)
                    : "cc"
                );
                
                // More inline asm with different constraints
                asm volatile (
                    "mul %0, %1, %2"
                    : "=r"(var12)
                    : "r"(var10), "r"(var11)
                );
                
                // Force output reload with volatile store
                vsink1 = var12;
                
                // Complex struct member access
                var13 = structs[idx1].data[idx2 % 16] + 
                        structs[idx2].data[idx3 % 16];
                
                // Pointer arithmetic forcing address reload
                int *temp_ptr = ptr1 + idx1;
                var14 = *temp_ptr + *(ptr2 + idx2);
                
                // Mixed type inline asm (potential secondary reload)
                asm volatile (
                    "fmov %s0, %w1\n\t"
                    "fcvt %d1, %s0"
                    : "=w"(fvar4), "=r"(var15)
                    : "w"(fvar3), "r"(var13)
                );
                
                // Another volatile store for output reload
                vsink2 = var15;
                
                // Complex assignment to array element (output reload)
                matrix1[(i + j) % SIZE][(i * j) % SIZE] = var14 + var15;
                
                // More variable chaining
                var16 = var13 + var14;
                var17 = var15 * 2;
                var18 = var16 - var17;
                var19 = var18 / 3;
                var20 = var19 + cvar1;
                
                // Function call with many register arguments
                var1 = helper_func(var16, var17, var18, var19, var20, fvar4, dvar3);
                
                // Final complex computation
                dvar4 = dvar2 + dmatrix[idx1][idx2] * 1.5;
                fvar5 = fvar3 + (float)dvar4;
                
                // More inline asm with memory constraint
                asm volatile (
                    "ldr %0, [%1, %2, lsl #2]\n\t"
                    "add %0, %0, %3"
                    : "=r"(var2)
                    : "r"(ptr1), "r"(idx1), "r"(var1)
                );
                
                // Volatile store forcing output reload
                vsink3 = var2;
                
                // Update checksum
                checksum += var1 + var2 + (int)fvar5;
            }
        }
        
        // Additional complex operations outside loops
        for (int k = 0; k < 16; k++) {
            // Complex addressing with struct pointer chain
            int offset = k * 3;
            var3 = structs[offset % SIZE].data[(offset * 7) % 16];
            
            // Inline asm with specific register class constraint
            register int reg_var asm("r12") = var3;
            asm volatile (
                "mov %0, %1\n\t"
                "lsl %0, %0, #2"
                : "=r"(var4)
                : "r"(reg_var)
            );
            
            // Builtin function that may need specific registers
            var5 = __builtin_popcount(var4);
            
            // Mixed type operation
            dvar1 = dvar1 + (double)var5 * 0.01;
            
            // Store to array with computed index
            matrix2[k][(k * 5) % SIZE] = var4 + var5;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

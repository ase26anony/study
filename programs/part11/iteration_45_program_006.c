#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 128
#define N_VARS 25

// Helper function that forces register usage for arguments
__attribute__((noinline, target("general-regs-only")))
int helper_func(int a, int b, int c, int d, int e, 
                float f, float g, double h, double i, char j) {
    volatile int result;
    // Force register pressure with mixed operations
    result = (a * b) + (c / (d + 1)) - (e << 2);
    result += (int)(f * g) + (int)h * (int)i;
    result += j * 256;
    
    // Inline asm with register constraints
    asm volatile (
        "add %0, %1, %2\n\t"
        "sub %0, %0, %3"
        : "=r"(result)
        : "r"(result), "r"(a), "r"(b)
        : "cc"
    );
    
    return result;
}

struct DataNode {
    int data[SIZE];
    double values[SIZE];
    struct DataNode* next;
    char metadata[64];
};

int main() {
    // Initialize complex data structures
    int matrix1[SIZE][SIZE];
    int matrix2[SIZE][SIZE];
    double matrix3[SIZE][SIZE];
    struct DataNode nodes[4];
    volatile int sink = 0;
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix1[i][j] = i * SIZE + j;
            matrix2[i][j] = (i + j) * 2;
            matrix3[i][j] = (double)(i * j) / 3.14159;
        }
    }
    
    for (int n = 0; n < 4; n++) {
        for (int i = 0; i < SIZE; i++) {
            nodes[n].data[i] = n * SIZE + i;
            nodes[n].values[i] = (double)(n * i) / 2.71828;
        }
        if (n < 3) nodes[n].next = &nodes[n + 1];
        else nodes[n].next = &nodes[0];
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix1, matrix2, matrix3, nodes) \
                       map(tofrom: checksum, sink)
    {
        // Declare many local variables to create register pressure
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        int var11, var12, var13, var14, var15, var16, var17, var18, var19, var20;
        float fvar1, fvar2, fvar3, fvar4, fvar5;
        double dvar1, dvar2, dvar3, dvar4;
        char cvar1, cvar2, cvar3, cvar4, cvar5;
        
        // Initialize from mapped arrays with complex addressing
        var1 = matrix1[0][0];
        var2 = matrix2[0][0];
        dvar1 = matrix3[0][0];
        
        // Complex nested loops with register-intensive computations
        for (int i = 1; i < 16; i++) {
            for (int j = 1; j < 16; j++) {
                // Complex array indexing forcing address reloads
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 7) % SIZE;
                int idx3 = (i * 11 + j * 19) % SIZE;
                
                // Chain computations across many variables
                var3 = matrix1[idx1][idx2] + matrix2[idx2][idx3];
                var4 = matrix2[idx3][idx1] * matrix1[idx2][idx1];
                
                // Pointer chain access forcing address computation
                struct DataNode* current = &nodes[i % 4];
                var5 = current->data[idx1] + current->next->data[idx2];
                dvar2 = current->values[idx3] * current->next->values[idx1];
                
                // Mixed type operations forcing mode conversions
                fvar1 = (float)var3 / (float)(var4 + 1);
                fvar2 = (float)dvar2 * 2.5f;
                
                // More variable chaining
                var6 = var3 + var4;
                var7 = var5 - var6;
                var8 = var7 * 3;
                var9 = var8 / (var6 + 1);
                var10 = var9 << 2;
                
                // Floating point operations
                fvar3 = fvar1 + fvar2;
                fvar4 = fvar3 * 1.414f;
                fvar5 = fvar4 - fvar1;
                
                // Double precision operations
                dvar3 = dvar1 + dvar2;
                dvar4 = dvar3 * 3.14159;
                
                // Character operations
                cvar1 = (char)(var6 & 0xFF);
                cvar2 = (char)(var7 & 0xFF);
                cvar3 = cvar1 + cvar2;
                cvar4 = cvar3 * 2;
                cvar5 = cvar4 - cvar1;
                
                // Inline assembly with register constraints
                // Force specific register usage
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "mul %0, %0, %3"
                    : "=r"(var11)
                    : "r"(var6), "r"(var7), "r"(var8)
                    : "cc"
                );
                
                // Another asm with different constraints
                asm volatile (
                    "fadd %s0, %s1, %s2\n\t"
                    "fmul %s0, %s0, %s3"
                    : "=w"(fvar5)
                    : "w"(fvar1), "w"(fvar2), "w"(fvar3)
                );
                
                // Force output reload with volatile store
                sink = var11 + (int)fvar5;
                
                // Complex assignment to array element (forces out reload)
                matrix1[(i + j) % SIZE][(i * j) % SIZE] = 
                    var11 + (int)(dvar4 * 100.0) + cvar5;
                
                // More variable chaining to keep them live
                var12 = var11 + var10;
                var13 = var12 - var9;
                var14 = var13 * var8;
                var15 = var14 / (var7 + 1);
                var16 = var15 << 1;
                var17 = var16 + var6;
                var18 = var17 - var5;
                var19 = var18 * var4;
                var20 = var19 / (var3 + 1);
                
                // Call helper function with many register arguments
                int func_result = helper_func(
                    var11, var12, var13, var14, var15,
                    fvar1, fvar2, (float)dvar3, (float)dvar4, cvar5
                );
                
                // Update checksum
                checksum += func_result + var20 + (int)(fvar5 * 10.0f);
                
                // Rotate variables to extend live ranges
                var1 = var2; var2 = var3; var3 = var4; var4 = var5;
                var5 = var6; var6 = var7; var7 = var8; var8 = var9;
                var9 = var10; var10 = var11; var11 = var12; var12 = var13;
                var13 = var14; var14 = var15; var15 = var16; var16 = var17;
                var17 = var18; var18 = var19; var19 = var20;
                
                fvar1 = fvar2; fvar2 = fvar3; fvar3 = fvar4; fvar4 = fvar5;
                dvar1 = dvar2; dvar2 = dvar3; dvar3 = dvar4;
                cvar1 = cvar2; cvar2 = cvar3; cvar3 = cvar4; cvar4 = cvar5;
            }
        }
        
        // Final complex computation
        int final_idx = (checksum * 13 + 17) % SIZE;
        struct DataNode* final_node = &nodes[checksum % 4];
        
        // Force one more address reload with complex expression
        checksum += final_node->data[final_idx] + 
                   final_node->next->data[(final_idx * 7) % SIZE] +
                   (int)final_node->values[(final_idx * 11) % SIZE];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Sink value: %d\n", sink);
    
    return 0;
}

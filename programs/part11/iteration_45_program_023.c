#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 128
#define NUM_VARS 20

/* Complex struct to force address computations */
struct DataNode {
    int values[SIZE];
    double fp_values[SIZE];
    struct DataNode *next;
    char metadata[64];
};

/* Non-inline helper to force register usage for arguments */
__attribute__((noinline)) 
int compute_checksum(int a, int b, int c, int d, int e, 
                     float f, double g, char h) {
    volatile int result;
    /* Force register pressure with inline asm */
    asm volatile (
        "add %w0, %w1, %w2\n\t"
        "add %w0, %w0, %w3\n\t"
        "add %w0, %w0, %w4"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d)
        : "cc"
    );
    return result + (int)f + (int)g + (int)h + e;
}

int main() {
    /* Initialize complex data structures */
    int matrix[SIZE][SIZE];
    double arr3d[SIZE][SIZE][SIZE];
    struct DataNode nodes[SIZE];
    volatile int sink; /* For forcing output reloads */
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
        for (int j = 0; j < SIZE; j++) {
            for (int k = 0; k < SIZE; k++) {
                arr3d[i][j][k] = (i * 1.5 + j * 2.0 + k * 0.5) / SIZE;
            }
        }
        nodes[i].next = (i < SIZE - 1) ? &nodes[i + 1] : NULL;
        for (int j = 0; j < SIZE; j++) {
            nodes[i].values[j] = i ^ j;
            nodes[i].fp_values[j] = (double)(i * j) / 1000.0;
        }
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, arr3d, nodes) map(from: checksum)
    {
        /* Declare many local variables to create register pressure */
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        float fvar1, fvar2, fvar3, fvar4, fvar5;
        double dvar1, dvar2, dvar3, dvar4;
        char cvar1, cvar2, cvar3;
        long lvar1, lvar2;
        
        /* Initialize from mapped arrays with complex addressing */
        var1 = matrix[0][0];
        var2 = matrix[SIZE/2][SIZE/3];
        var3 = nodes[10].values[20];
        
        /* Complex pointer chain access */
        struct DataNode *node_ptr = &nodes[0];
        var4 = node_ptr->next->next->values[15];
        
        /* Force address computation in registers */
        int stride = SIZE;
        int offset = 7;
        
        /* Nested loops creating live range interference */
        for (int i = 1; i < 8; i++) {
            for (int j = 1; j < 8; j++) {
                for (int k = 1; k < 8; k++) {
                    /* Complex array indexing forcing address reloads */
                    int idx1 = (i * stride + j * 3 + k) % SIZE;
                    int idx2 = (j * stride + k * 5 + offset) % SIZE;
                    int idx3 = (k * stride + i * 7 + offset * 2) % SIZE;
                    
                    /* Load with complex addressing modes */
                    var5 = matrix[idx1][idx2];
                    var6 = nodes[idx3].values[idx1];
                    
                    /* Mixed-type computations forcing mode conversions */
                    fvar1 = (float)var5 * 1.5f + (float)var6 * 0.5f;
                    dvar1 = (double)fvar1 * arr3d[i][j][k];
                    
                    /* Inline assembly with register constraints */
                    int temp1, temp2;
                    asm volatile (
                        "mul %w0, %w1, %w2\n\t"
                        "add %w0, %w0, #1"
                        : "=r"(temp1)
                        : "r"(var5), "r"(var6)
                        : "cc"
                    );
                    
                    /* Another asm with different constraints */
                    double ftemp;
                    asm volatile (
                        "fcvt %d0, %s1\n\t"
                        "fmul %d0, %d0, %d0"
                        : "=w"(ftemp)
                        : "w"(fvar1)
                    );
                    
                    /* Force output reload with volatile store */
                    sink = temp1;
                    
                    /* Complex assignment to array element (output reload) */
                    int *dyn_ptr = &matrix[idx1][idx2];
                    *dyn_ptr = temp1 + (int)ftemp;  /* out operand */
                    
                    /* Chain computations to keep variables live */
                    var7 = var1 + var2;
                    var8 = var3 * var4;
                    var9 = var5 ^ var6;
                    var10 = var7 - var8;
                    
                    fvar2 = fvar1 * 2.0f;
                    fvar3 = fvar2 + (float)var9;
                    fvar4 = fvar3 / 3.14f;
                    fvar5 = fvar4 - fvar2;
                    
                    dvar2 = dvar1 + (double)fvar5;
                    dvar3 = dvar2 * arr3d[idx1][idx2][idx3];
                    dvar4 = dvar3 / 2.71828;
                    
                    cvar1 = (char)(var10 & 0xFF);
                    cvar2 = cvar1 + (char)i;
                    cvar3 = cvar2 - (char)j;
                    
                    lvar1 = (long)var10 * 1000L;
                    lvar2 = lvar1 + (long)(dvar4 * 1000.0);
                    
                    /* Secondary reload trigger: mixed register classes */
                    int int_from_fp;
                    asm volatile (
                        "fcvtzs %w0, %s1"
                        : "=r"(int_from_fp)
                        : "w"(fvar3)  /* 'w' constraint for FP register */
                    );
                    
                    /* More register pressure with builtins */
                    var1 = __builtin_popcount(var10);
                    var2 = __builtin_ffs(var9);
                    
                    /* Function call forcing argument passing in registers */
                    int func_result = compute_checksum(
                        var1, var2, var3, var4, var5,
                        fvar1, dvar1, cvar1
                    );
                    
                    /* Complex store with addressing */
                    nodes[k].values[(i * j) % SIZE] = func_result + int_from_fp;
                    
                    /* Update checksum */
                    checksum += var1 + var2 + var3 + var4 + var5 +
                               var6 + var7 + var8 + var9 + var10 +
                               (int)fvar1 + (int)fvar2 + (int)fvar3 +
                               (int)fvar4 + (int)fvar5 +
                               (int)dvar1 + (int)dvar2 + (int)dvar3 +
                               (int)dvar4 + cvar1 + cvar2 + cvar3 +
                               (int)(lvar1 % 100) + (int)(lvar2 % 100);
                }
            }
        }
        
        /* Final volatile store to force output reload */
        volatile int *volatile_ptr = &sink;
        *volatile_ptr = checksum;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

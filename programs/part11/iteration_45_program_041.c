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

/* Non-inline helper to force register passing */
__attribute__((noinline)) 
int compute_checksum(int a, int b, int c, int d, int e, 
                     float f, double g, char h, int i, int j) {
    volatile int sink;
    /* Force register pressure in helper too */
    asm volatile ("# Helper start" : : : "memory");
    int result = (a * b) + (c - d) + (int)(f * 10) + (int)g + h + i * j;
    sink = result;  /* Volatile store */
    asm volatile ("# Helper end" : : : "memory");
    return result;
}

int main() {
    /* Initialize complex data structures */
    int matrix[SIZE][SIZE];
    double arr_3d[SIZE][SIZE][SIZE];
    struct DataNode nodes[SIZE];
    volatile int global_sink;
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
            for (int k = 0; k < SIZE; k++) {
                arr_3d[i][j][k] = (i * 1.5 + j * 2.3 + k * 0.7) / SIZE;
            }
        }
        nodes[i].next = (i < SIZE-1) ? &nodes[i+1] : NULL;
        for (int j = 0; j < SIZE; j++) {
            nodes[i].values[j] = i ^ j;
            nodes[i].fp_values[j] = (i * j) / 100.0;
        }
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, arr_3d, nodes) map(from: checksum)
    {
        /* Declare many local variables to create register pressure */
        register int var0, var1, var2, var3, var4, var5, var6, var7, var8, var9;
        float fvar0, fvar1, fvar2, fvar3, fvar4;
        double dvar0, dvar1, dvar2, dvar3;
        char cvar0, cvar1, cvar2;
        volatile int vsink;
        volatile double vdsink;
        
        /* Initialize from mapped arrays with complex addressing */
        var0 = matrix[0][0];
        var1 = matrix[SIZE/4][SIZE/2];
        var2 = matrix[SIZE/2][SIZE/4];
        
        /* Complex array indexing forcing address reloads */
        for (int i = 1; i < SIZE/8; i++) {
            int stride = SIZE/16;
            for (int j = 1; j < SIZE/8; j++) {
                /* Force many live variables */
                var3 = matrix[(i*stride + j) % SIZE][(j*stride + i) % SIZE];
                var4 = matrix[(i*stride * 3 + j*7) % SIZE][(j*stride * 2 + i*5) % SIZE];
                
                /* Mixed type computations */
                fvar0 = (float)var3 * 0.5f + (float)var4 * 0.3f;
                fvar1 = fvar0 * 2.0f - (float)(i * j);
                
                /* Complex pointer chain access */
                struct DataNode *node_ptr = &nodes[i];
                var5 = node_ptr->values[(i + j) % SIZE];
                var6 = node_ptr->next ? node_ptr->next->values[(i * j) % SIZE] : 0;
                
                /* Multi-dimensional array with complex index */
                dvar0 = arr_3d[i][j][(i + j) % SIZE];
                dvar1 = arr_3d[j][i][(i * j) % SIZE];
                
                /* Inline assembly with register constraints */
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %3, %4, %5"
                    : "=r"(var7), "+r"(var8)
                    : "r"(var3), "r"(var4), "r"(var5), "r"(var6)
                    : "cc"
                );
                
                /* More assembly with mixed constraints */
                int temp1, temp2;
                asm volatile (
                    "mov %0, %1\n\t"
                    "fmov %s2, %w3"
                    : "=r"(temp1), "=w"(fvar2)
                    : "r"(var7), "r"(var8)
                );
                
                /* Force output reload with volatile store */
                vsink = var7 + var8;
                
                /* Complex assignment to array element (needs address computation) */
                int *dyn_ptr = &matrix[i][j];
                *dyn_ptr = var7 * var8 - var5 + var6;
                
                /* Another volatile store with complex expression */
                vdsink = dvar0 * dvar1 + (double)fvar0 - (double)var3;
                
                /* Chain computations keeping many variables live */
                var9 = var0 + var1 - var2 + var3 * var4 / (var5 + 1);
                cvar0 = (char)(var9 % 256);
                cvar1 = (char)((var9 * 3) % 256);
                cvar2 = cvar0 ^ cvar1;
                
                /* More register pressure variables */
                fvar3 = fvar1 * 3.14f + (float)cvar2;
                fvar4 = fvar2 - fvar3 * 0.5f;
                dvar2 = dvar0 + dvar1 * 2.0;
                dvar3 = dvar2 / (fvar4 + 1.0);
                
                /* Function call forcing register arguments */
                int call_result = compute_checksum(
                    var0, var1, var2, var3, var4,
                    fvar0, dvar0, cvar0, var7, var8
                );
                
                /* Use result in complex expression */
                var0 = var1 + call_result;
                var1 = var2 * call_result - var0;
                
                /* More inline assembly with specific constraints */
                double fp_result;
                asm volatile (
                    "fmul %d0, %d1, %d2\n\t"
                    "fcvt %s3, %d0"
                    : "=w"(fp_result), "=w"(fvar4)
                    : "w"(dvar2), "w"(dvar3)
                );
                
                /* Final assignment with complex addressing */
                nodes[i].values[j] = var0 + var1 + (int)fp_result;
                nodes[i].fp_values[j] = fp_result + dvar3;
                
                /* Update checksum */
                checksum += var0 + var1 + (int)fvar0 + (int)dvar0 + cvar0;
            }
        }
        
        /* Additional computations outside loops */
        for (int i = 0; i < NUM_VARS; i++) {
            /* Force all variables to be used */
            asm volatile (
                "# Using all vars %0 %1 %2 %3 %4 %5 %6 %7"
                : 
                : "r"(var0), "r"(var1), "r"(var2), "r"(var3),
                  "r"(var4), "r"(var5), "r"(var6), "r"(var7)
            );
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

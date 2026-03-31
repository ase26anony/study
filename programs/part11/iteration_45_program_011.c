#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define ITERS 16

/* Complex struct to force address computations */
struct DataNode {
    int values[4];
    double fp_data[3];
    struct DataNode *next;
    char metadata[8];
};

/* Non-inline helper to force register usage for arguments */
__attribute__((noinline)) 
int compute_checksum(int a, int b, int c, double d, float e, char f) {
    volatile int result;
    /* Force register pressure with inline asm */
    asm volatile (
        "add %w0, %w1, %w2\n\t"
        "add %w0, %w0, %w3\n\t"
        "scvtf %s4, %w0\n\t"
        "fadd %s4, %s4, %s5"
        : "=r"(result), "+w"(d)
        : "r"(a), "r"(b), "r"(c), "w"(e), "r"(f)
        : "cc"
    );
    return result;
}

int main() {
    /* Multi-dimensional arrays with complex access patterns */
    int matrix[SIZE][SIZE];
    double arr3d[ITERS][SIZE/2][SIZE/2];
    struct DataNode nodes[SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * 17 + j * 13) % 256;
        }
        for (int j = 0; j < SIZE/2; j++) {
            for (int k = 0; k < SIZE/2; k++) {
                arr3d[i % ITERS][j][k] = (i + j * 1.5 + k * 2.7) / 100.0;
            }
        }
        nodes[i].values[0] = i;
        nodes[i].values[1] = i * 2;
        nodes[i].values[2] = i * 3;
        nodes[i].values[3] = i * 4;
        nodes[i].fp_data[0] = i * 0.1;
        nodes[i].fp_data[1] = i * 0.2;
        nodes[i].fp_data[2] = i * 0.3;
        nodes[i].next = &nodes[(i + 1) % SIZE];
        strncpy(nodes[i].metadata, "DATA", 8);
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, arr3d, nodes) map(from: checksum)
    {
        /* Create massive register pressure with many local variables */
        register int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        register float f1, f2, f3, f4, f5, f6, f7, f8;
        register double d1, d2, d3, d4, d5;
        register char c1, c2, c3, c4;
        volatile int sink1, sink2, sink3;
        volatile double dsink;
        
        /* Initialize from mapped arrays with complex addressing */
        v1 = matrix[0][0];
        v2 = matrix[1][SIZE-1];
        v3 = matrix[SIZE/4][SIZE/2];
        v4 = matrix[SIZE/2][SIZE/4];
        
        /* Complex array indexing forcing address reloads */
        for (int iter = 0; iter < ITERS; iter++) {
            int stride = (iter * 7) % 13 + 1;
            int offset = (iter * 11) % 17;
            
            /* Force many live variables simultaneously */
            v5 = matrix[(iter * stride + offset) % SIZE][(offset * 3) % SIZE];
            v6 = matrix[(iter * 5 + stride) % SIZE][(stride * 7) % SIZE];
            
            /* Multi-dimensional array with complex index */
            d1 = arr3d[iter % ITERS][(iter + stride) % (SIZE/2)][(offset * 2) % (SIZE/2)];
            d2 = arr3d[(iter + 5) % ITERS][(stride * 3) % (SIZE/2)][(offset + 7) % (SIZE/2)];
            
            /* Struct member access with pointer chain - forces address computation */
            struct DataNode *node_ptr = &nodes[iter % SIZE];
            v7 = node_ptr->values[iter % 4];
            v8 = node_ptr->next->values[(iter + 1) % 4];
            v9 = node_ptr->next->next->values[(iter + 2) % 4];
            
            d3 = node_ptr->fp_data[iter % 3];
            d4 = node_ptr->next->fp_data[(iter + 1) % 3];
            
            /* Mixed type computations forcing mode conversions */
            f1 = (float)d1 + (float)v5 * 0.5f;
            f2 = (float)d2 + (float)v6 * 0.3f;
            f3 = f1 * f2 - (float)v7;
            f4 = f3 / (f2 + 1.0f);
            
            /* Inline assembly with register constraints to force reloads */
            asm volatile (
                "add %w0, %w1, %w2\n\t"
                "mul %w0, %w0, %w3\n\t"
                "add %w0, %w0, %w4"
                : "=r"(v10)
                : "r"(v5), "r"(v6), "r"(v7), "r"(v8)
                : "cc"
            );
            
            /* More inline asm with mixed register classes */
            asm volatile (
                "scvtf %s0, %w1\n\t"
                "fadd %s0, %s0, %s2\n\t"
                "fmul %s0, %s0, %s3"
                : "=w"(f5)
                : "r"(v9), "w"(f3), "w"(f4)
            );
            
            /* Force output reload with volatile store */
            sink1 = v10;
            sink2 = (int)f5;
            
            /* Complex pointer dereference for output reload */
            int * volatile ptr_out = &matrix[iter % SIZE][(iter * 3) % SIZE];
            *ptr_out = v10 + (int)f5;
            
            /* Another output with computed index */
            matrix[(iter * 2) % SIZE][(iter * 5) % SIZE] = 
                node_ptr->values[0] + node_ptr->values[1] + 
                node_ptr->next->values[2];
            
            /* Force secondary reloads with specific register constraints */
            #ifdef __aarch64__
            /* Move between general and FP registers */
            asm volatile (
                "fmov %s0, %w1\n\t"
                "fadd %s0, %s0, %s2"
                : "=w"(f6)
                : "r"(v10), "w"(f1)
            );
            #else
            /* x86 version forcing specific registers */
            asm volatile (
                "cvtsi2ss %1, %0\n\t"
                "addss %0, %2"
                : "=x"(f6)
                : "r"(v10), "x"(f1)
            );
            #endif
            
            /* Builtin functions that use specific registers */
            c1 = (char)(iter & 0xFF);
            c2 = (char)((iter >> 8) & 0xFF);
            c3 = __builtin_popcount(iter);
            c4 = __builtin_parity(iter);
            
            /* More register pressure with chained computations */
            v1 = v1 + v2 * v3 - v4;
            v2 = v2 + v3 * v4 - v5;
            v3 = v3 + v4 * v5 - v6;
            v4 = v4 + v5 * v6 - v7;
            
            f7 = f1 + f2 * f3 - f4;
            f8 = f2 + f3 * f4 - f5;
            
            d5 = d1 + d2 * d3 - d4;
            
            /* Force function call with register arguments */
            int call_result = compute_checksum(v1, v2, v3, d5, f7, c1);
            
            /* Complex assignment to volatile */
            dsink = d5 + (double)call_result;
            
            /* Update checksum with complex expression */
            checksum += v10 + (int)f6 + call_result + 
                       matrix[iter % SIZE][(iter + 1) % SIZE] +
                       (int)(node_ptr->fp_data[0] * 100.0);
        }
        
        /* Final volatile stores to ensure all values are used */
        sink3 = v1 + v2 + v3 + v4;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

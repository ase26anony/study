#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 128
#define NUM_VARS 25

/* Struct to force complex addressing */
struct DataNode {
    int values[4];
    double fp_data[3];
    struct DataNode *next;
    char metadata[16];
};

/* Non-inline helper to force register usage for arguments */
__attribute__((noinline)) 
int compute_checksum(int a, int b, int c, double d, float e, char f) {
    volatile int result;
    /* Force register pressure with inline asm */
    asm volatile (
        "add %w0, %w1, %w2\n\t"
        "add %w0, %w0, %w3\n\t"
        "fcvt s2, %d4\n\t"
        "fcvt s3, %s5\n\t"
        "fcvtzs w4, s2\n\t"
        "add %w0, %w0, w4\n\t"
        "fcvtzs w5, s3\n\t"
        "add %w0, %w0, w5\n\t"
        "sxtb w6, %w6\n\t"
        "add %w0, %w0, w6"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f)
        : "w2", "w3", "w4", "w5", "w6", "s2", "s3"
    );
    return result;
}

int main() {
    /* Multi-dimensional arrays with padding */
    int matrix[SIZE][SIZE + 3];
    double arr_3d[32][32][32];
    char byte_grid[SIZE][SIZE * 2];
    
    /* Struct array with pointer chain */
    struct DataNode nodes[SIZE];
    volatile int sink; /* For forcing output reloads */
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE + 3; j++) {
            matrix[i][j] = (i * 17 + j * 13) % 256;
        }
        for (int j = 0; j < SIZE * 2; j++) {
            byte_grid[i][j] = (i + j) % 128;
        }
        nodes[i].values[0] = i * 3;
        nodes[i].values[1] = i * 5;
        nodes[i].values[2] = i * 7;
        nodes[i].values[3] = i * 11;
        nodes[i].fp_data[0] = i * 1.1;
        nodes[i].fp_data[1] = i * 2.2;
        nodes[i].fp_data[2] = i * 3.3;
        nodes[i].next = (i < SIZE - 1) ? &nodes[i + 1] : NULL;
        for (int k = 0; k < 16; k++) {
            nodes[i].metadata[k] = (i + k) % 26 + 'A';
        }
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 32; k++) {
                arr_3d[i][j][k] = (i * j * k) * 0.01;
            }
        }
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, arr_3d, byte_grid, nodes) \
                       map(tofrom: checksum)
    {
        /* Declare many local variables to consume registers */
        register int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
        float f0, f1, f2, f3, f4, f5, f6, f7;
        double d0, d1, d2, d3, d4;
        char c0, c1, c2, c3;
        uint64_t u0, u1;
        
        /* Initialize from mapped arrays with complex addressing */
        v0 = matrix[0][0];
        v1 = matrix[1][(SIZE + 2) % (SIZE + 3)];
        v2 = matrix[2][(2 * SIZE) % (SIZE + 3)];
        v3 = matrix[3][(3 * SIZE + 1) % (SIZE + 3)];
        
        /* Force address computation in registers */
        int idx1 = (v0 * 17 + v1 * 13) % SIZE;
        int idx2 = (v2 * 19 + v3 * 7) % SIZE;
        
        /* Complex array access with multi-dimensional indexing */
        d0 = arr_3d[idx1 % 32][idx2 % 32][(idx1 + idx2) % 32];
        d1 = arr_3d[(idx1 * 2) % 32][(idx2 / 2) % 32][(idx1 * idx2) % 32];
        
        /* Struct member access with pointer chain */
        struct DataNode *node_ptr = &nodes[idx1];
        v4 = node_ptr->values[0];
        v5 = node_ptr->values[(idx2 % 3) + 1];
        d2 = node_ptr->fp_data[idx2 % 3];
        
        /* More complex addressing with struct pointer arithmetic */
        if (node_ptr->next) {
            v6 = node_ptr->next->values[(idx1 + idx2) % 4];
            d3 = node_ptr->next->fp_data[(idx1 * idx2) % 3];
        }
        
        /* Byte array with stride access */
        c0 = byte_grid[idx1][idx2 * 2];
        c1 = byte_grid[idx2][idx1 * 3 % (SIZE * 2)];
        
        /* Start chaining computations - all variables live simultaneously */
        f0 = (float)v0 * 0.5f + (float)c0;
        f1 = (float)v1 * 1.5f + (float)c1;
        f2 = (float)v2 * 2.5f + f0;
        f3 = (float)v3 * 3.5f + f1;
        
        /* Mixed type operations forcing conversions */
        d4 = d0 + (double)f2 + (double)v4;
        f4 = (float)d1 + f3 + (float)v5;
        
        /* Inline assembly with register constraints */
        asm volatile (
            "add %w0, %w1, %w2\n\t"
            "sub %w3, %w4, %w5\n\t"
            "mul %w6, %w0, %w3"
            : "=r"(v7), "=r"(v8), "=r"(v9)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "0"(v4), "1"(v5), "2"(v6)
            : "cc"
        );
        
        /* More inline asm with floating point constraints */
        asm volatile (
            "fadd %s0, %s1, %s2\n\t"
            "fcvt %d3, %s0"
            : "=w"(f5), "=w"(f6), "=r"(u0)
            : "w"(f0), "w"(f1), "r"(v7)
            : 
        );
        
        /* Secondary reload scenario: move between register classes */
        asm volatile (
            "fmov %s0, %w1\n\t"  /* Move from general to FP register */
            "fcvt %d2, %s0"
            : "=w"(f7), "=r"(u1)
            : "r"(v8), "r"(v9)
            : 
        );
        
        /* Builtin that may require specific registers */
        v8 = __builtin_popcount(v7 | v8);
        v9 = __builtin_clz(v9);
        
        /* Complex loop with register pressure */
        for (int i = 0; i < 16; i++) {
            /* Compute complex index */
            int arr_idx = (i * idx1 + idx2) % SIZE;
            int byte_idx = (i * 3 + idx1 * 7) % (SIZE * 2);
            
            /* Force output reload: assignment to volatile */
            sink = matrix[arr_idx][(byte_idx / 2) % (SIZE + 3)];
            
            /* Assignment to array element with computed index */
            byte_grid[arr_idx % SIZE][byte_idx] = 
                (v7 + v8 + i) % 128;
            
            /* More mixed computations keeping variables live */
            f0 = f0 + (float)sink * 0.1f;
            f1 = f1 + (float)byte_grid[arr_idx % SIZE][byte_idx] * 0.2f;
            d0 = d0 + (double)f0 * 0.01;
            d1 = d1 + (double)f1 * 0.02;
            
            /* Pointer dereference assignment forcing output reload */
            int *temp_ptr = &matrix[arr_idx][(i * 5) % (SIZE + 3)];
            *temp_ptr = v9 + i + (int)d0;
            
            /* Chain all variables together */
            v0 = v0 + v1 + v2;
            v1 = v1 + v3 + v4;
            v2 = v2 + v5 + v6;
            v3 = v3 + v7 + v8;
            v4 = v4 + v9 + sink;
        }
        
        /* Call helper function with many register arguments */
        checksum = compute_checksum(v0, v1, v2, d2, f2, c0);
        checksum += compute_checksum(v3, v4, v5, d3, f3, c1);
        
        /* Final complex expression with all variables */
        checksum += (int)((d0 + d1 + d2 + d3 + d4) * 1000);
        checksum += (int)((f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7) * 100);
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += c0 + c1;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}

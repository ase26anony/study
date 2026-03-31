#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 128
#define ITERS 100

/* Struct to create pointer chains */
struct DataNode {
    int values[4];
    double fp_data[2];
    struct DataNode *next;
    char metadata[8];
};

/* Non-inline helper to force register usage for arguments */
__attribute__((noinline)) 
int compute_checksum(int a, int b, int c, int d, int e, float f, double g) {
    volatile int sink;
    sink = a + b - c * d + e;
    sink += (int)(f * 100.0f);
    sink += (int)(g * 10.0);
    return sink;
}

int main() {
    /* Multi-dimensional arrays with complex access patterns */
    int matrix[SIZE][SIZE];
    double arr3d[32][32][32];
    char byte_grid[SIZE][SIZE];
    struct DataNode nodes[SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
            byte_grid[i][j] = (i + j) % 256;
        }
        nodes[i].values[0] = i * 2;
        nodes[i].values[1] = i * 3;
        nodes[i].values[2] = i * 4;
        nodes[i].values[3] = i * 5;
        nodes[i].fp_data[0] = i * 1.5;
        nodes[i].fp_data[1] = i * 2.5;
        nodes[i].next = (i < SIZE - 1) ? &nodes[i + 1] : &nodes[0];
        for (int k = 0; k < 8; k++) {
            nodes[i].metadata[k] = (i + k) % 128;
        }
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 32; k++) {
                arr3d[i][j][k] = (i * 1024 + j * 32 + k) * 0.01;
            }
        }
    }
    
    int result = 0;
    
    #pragma omp target map(to: matrix[0:SIZE][0:SIZE], \
                                 arr3d[0:32][0:32][0:32], \
                                 byte_grid[0:SIZE][0:SIZE], \
                                 nodes[0:SIZE]) \
                      map(tofrom: result)
    {
        /* Many scalar variables to create register pressure */
        register int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        float f1, f2, f3, f4, f5, f6, f7, f8;
        double d1, d2, d3, d4, d5;
        char c1, c2, c3, c4, c5;
        volatile int vsink1, vsink2, vsink3;
        volatile double vdsink;
        
        /* Initialize from mapped arrays with complex addressing */
        v1 = matrix[0][0];
        v2 = matrix[SIZE-1][SIZE-1];
        v3 = byte_grid[1][1] + byte_grid[2][2];
        v4 = nodes[0].values[0] + nodes[0].values[1];
        v5 = nodes[SIZE/2].values[2] * nodes[SIZE/2].values[3];
        f1 = (float)arr3d[0][0][0];
        f2 = (float)arr3d[1][1][1];
        d1 = arr3d[2][2][2];
        d2 = arr3d[3][3][3];
        c1 = nodes[0].metadata[0];
        c2 = nodes[1].metadata[1];
        
        /* Complex loop with mixed operations */
        for (int iter = 0; iter < ITERS; iter++) {
            int i = iter % (SIZE - 10);
            int j = (iter * 7) % (SIZE - 10);
            int k = (iter * 13) % 30;
            
            /* Complex array indexing forcing address reloads */
            int idx1 = (i * SIZE + j + k) % SIZE;
            int idx2 = (j * SIZE + i - k + iter) % SIZE;
            int idx3d = (i * 1024 + j * 32 + k) % (32*32*32);
            
            /* Chain computations keeping many variables live */
            v6 = matrix[idx1][idx2] + v1;
            v7 = byte_grid[i][j] * v2 - v3;
            v8 = nodes[i].values[i % 4] + nodes[j].values[j % 4];
            v9 = v4 * v5 - v6 + v7;
            v10 = v8 ^ v9;
            
            /* Mixed type computations forcing mode conversions */
            f3 = f1 * (float)v6 + f2;
            f4 = (float)v7 / 2.0f + f3;
            f5 = f4 - (float)(c1 + c2);
            f6 = f5 * 0.5f;
            f7 = f6 + (float)d1;
            f8 = f7 - (float)(v10 % 100);
            
            d3 = d1 * 0.25 + d2;
            d4 = (double)v9 * 0.01 + d3;
            d5 = d4 + (double)f8;
            
            c3 = (char)(v6 % 256);
            c4 = (char)(v7 % 256) + c1;
            c5 = c2 + c3 - c4;
            
            /* Inline assembly with register constraints */
            int temp1, temp2;
            asm volatile (
                "add %0, %1, %2\n\t"
                "sub %3, %0, %4"
                : "=r"(temp1), "=r"(temp2)
                : "r"(v6), "0"(v7), "r"(v8)
                : "cc"
            );
            
            /* More assembly with mixed register classes */
            double ftemp;
            asm volatile (
                "fcvt %d0, %w1\n\t"
                "fadd %d0, %d0, %d2"
                : "=w"(ftemp)
                : "r"(temp1), "w"(d3)
                : 
            );
            
            /* Force output reloads with volatile and pointer derefs */
            vsink1 = v9 + temp1;
            vsink2 = v10 * 2;
            vdsink = d5 + ftemp;
            
            /* Complex struct access with pointer chain */
            struct DataNode *node_ptr = &nodes[i];
            for (int chain = 0; chain < 3; chain++) {
                node_ptr = node_ptr->next;
                vsink3 = node_ptr->values[chain % 4];
                
                /* Assignment to computed array index */
                int store_idx = (i + chain) % SIZE;
                matrix[store_idx][j] = vsink3 + iter;
            }
            
            /* 3D array access with complex index */
            int x = (i + iter) % 32;
            int y = (j * 2) % 32;
            int z = (k * 3) % 32;
            double val3d = arr3d[x][y][z];
            
            /* Function call forcing register usage for arguments */
            int checksum = compute_checksum(v6, v7, v8, v9, v10, f8, d5);
            
            /* Update result with complex expression */
            result += checksum + (int)val3d + temp2 + (int)ftemp;
            
            /* Rotate values to extend live ranges */
            v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
            f1 = f2; f2 = f3; f3 = f4; f4 = f5;
            d1 = d2; d2 = d3; d3 = d4; d4 = d5;
            c1 = c2; c2 = c3; c3 = c4; c4 = c5;
        }
        
        /* Final volatile store to prevent optimization */
        vsink1 = result;
    }
    
    printf("Result: %d\n", result);
    return 0;
}

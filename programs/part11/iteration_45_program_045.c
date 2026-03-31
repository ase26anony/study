#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define ITERS 100

/* Struct to create pointer chains */
struct DataNode {
    int data[SIZE];
    struct DataNode *next;
    double matrix[4][4];
    volatile int counter;
};

/* Non-inline helper to force register usage for arguments */
__attribute__((noinline)) 
int compute_helper(int a, int b, int c, float d, double e, char f) {
    volatile int result;
    /* Force register pressure with inline asm */
    asm volatile (
        "add %w0, %w1, %w2\n\t"
        "add %w0, %w0, %w3\n\t"
        "scvtf s1, %w0\n\t"
        "fadd s1, s1, %4.s[0]\n\t"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "w"(d), "r"(f), "r"(e)
        : "s1", "memory"
    );
    return result;
}

int main() {
    /* Multi-dimensional arrays with complex access patterns */
    int matrix_3d[SIZE][SIZE][SIZE];
    double large_array[SIZE * 4];
    char char_grid[SIZE][SIZE];
    float float_stream[SIZE * 2];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            for (int k = 0; k < SIZE; k++) {
                matrix_3d[i][j][k] = (i * 7919 + j * 65537 + k * 1009) % 256;
            }
            char_grid[i][j] = (i + j) % 128;
        }
        large_array[i] = i * 3.14159;
        float_stream[i] = i * 2.71828f;
    }
    
    /* Create struct with pointer chain */
    struct DataNode nodes[4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < SIZE; j++) {
            nodes[i].data[j] = (i * SIZE + j) * 7;
        }
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                nodes[i].matrix[x][y] = (x * 0.1 + y * 0.01) * i;
            }
        }
        nodes[i].next = (i < 3) ? &nodes[i + 1] : &nodes[0];
        nodes[i].counter = 0;
    }
    
    int result = 0;
    
    #pragma omp target map(to: matrix_3d, large_array, char_grid, float_stream, nodes) \
                       map(tofrom: result)
    {
        /* Declare many local variables to create register pressure */
        register int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
        register float f0, f1, f2, f3, f4, f5, f6, f7;
        register double d0, d1, d2, d3, d4;
        register char c0, c1, c2, c3, c4, c5;
        volatile int sink;
        volatile double dsink;
        
        /* Initialize from mapped arrays with complex addressing */
        r0 = matrix_3d[0][0][0];
        r1 = matrix_3d[SIZE/2][SIZE/3][SIZE/4];
        r2 = matrix_3d[SIZE-1][SIZE-1][SIZE-1];
        
        /* Complex array indexing with multiple computations */
        for (int iter = 0; iter < ITERS; iter++) {
            int i = (iter * 17) % SIZE;
            int j = (iter * 23) % SIZE;
            int k = (iter * 29) % SIZE;
            
            /* Force address computation into registers */
            int idx1 = (i * SIZE * SIZE + j * SIZE + k) % (SIZE * SIZE * SIZE);
            int idx2 = ((i * 7919 + j * 65537) % SIZE) * SIZE + k;
            int idx3 = (i + j * 3 + k * 7) % SIZE;
            
            /* Load with complex addressing - forces address reloads */
            r3 = matrix_3d[i][j][k];
            r4 = matrix_3d[(i * 3 + j) % SIZE][(j * 5 + k) % SIZE][(k * 7 + i) % SIZE];
            r5 = nodes[(i + j) % 4].data[(i * j + k) % SIZE];
            
            /* Mixed type computations */
            f0 = float_stream[idx1 % (SIZE * 2)];
            f1 = float_stream[idx2 % (SIZE * 2)];
            d0 = large_array[idx3];
            d1 = large_array[(idx1 + idx2) % (SIZE * 4)];
            
            /* Chain computations to extend live ranges */
            r6 = r0 + r1 * 2 - r2 / 3;
            r7 = r3 * r4 + r5;
            r8 = r6 ^ r7;
            r9 = (r8 << 3) | (r8 >> 5);
            r10 = r9 + iter;
            
            f2 = f0 * 1.5f + f1;
            f3 = f2 / 3.14f - f0;
            f4 = f3 * f1 + f2;
            f5 = f4 - f3 * 2.0f;
            f6 = f5 + (float)r10 * 0.01f;
            f7 = f6 * f2 - f4;
            
            d2 = d0 * 2.71828 + d1;
            d3 = d2 / 3.14159 - d0;
            d4 = d3 * 1.4142 + d2;
            
            c0 = char_grid[i][j];
            c1 = char_grid[j][k];
            c2 = char_grid[k][i];
            c3 = (c0 + c1 - c2) % 64;
            c4 = c3 * 2 + iter % 32;
            c5 = c4 ^ c0;
            
            /* Inline assembly with register constraints to force reloads */
            /* General purpose register pressure */
            asm volatile (
                "add %w0, %w1, %w2\n\t"
                "mul %w0, %w0, %w3\n\t"
                "sub %w0, %w0, %w4\n\t"
                : "=r"(r0)
                : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "0"(r0)
                : "cc"
            );
            
            /* Force output reload with volatile store */
            sink = r0 + r7;
            
            /* Mixed register class constraints (GPR vs FP) */
            asm volatile (
                "fmov %s0, %w1\n\t"      /* Move from GPR to FP register */
                "fcvt %d1, %s0\n\t"      /* Single to double precision */
                : "=w"(f0), "=w"(d0)
                : "r"(r8), "0"(f0), "1"(d0)
            );
            
            /* Memory constraint forcing address into register */
            int temp;
            asm volatile (
                "ldr %w0, [%1, %2, lsl #2]\n\t"
                : "=r"(temp)
                : "r"(nodes[0].data), "r"(idx1 % SIZE)
                : "memory"
            );
            
            /* Complex struct member access with pointer chain */
            double* matrix_ptr = nodes[i % 4].next->next->matrix[(i + j) % 4];
            dsink = matrix_ptr[(j + k) % 4] + d4;
            
            /* Assignment to array element with computed index - forces output reload */
            char_grid[(i * 3 + iter) % SIZE][(j * 5 + iter) % SIZE] = c5 + iter;
            
            /* Volatile struct member update */
            nodes[iter % 4].counter = iter;
            
            /* Call helper function with many register arguments */
            r10 = compute_helper(r0, r1, r2, f3, d2, c3);
            
            /* More arithmetic to keep variables live */
            r0 = r0 + r10;
            r1 = r1 * 2 - r10;
            f0 = f0 + (float)r10 * 0.01f;
            d0 = d0 + (double)r10 * 0.001;
            
            /* Store to memory with complex address computation */
            int* data_ptr = nodes[(i + j + k) % 4].next->data;
            data_ptr[(i * j + k * 3) % SIZE] = r0 + r1 + r10;
            
            /* Force secondary reloads with specific register class requirements */
            long long wide_val;
            asm volatile (
                "smull %x0, %w1, %w2\n\t"  /* Requires specific multiplier registers */
                : "=r"(wide_val)
                : "r"(r3), "r"(r4)
            );
            
            sink = (int)wide_val;
        }
        
        /* Final computation for result */
        result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10
                 + (int)f0 + (int)f1 + (int)f2 + (int)f3
                 + (int)d0 + (int)d1 + sink;
    }
    
    printf("Result: %d\n", result);
    return 0;
}

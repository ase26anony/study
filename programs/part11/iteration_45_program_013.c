#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define SIZE 128
#define ITERS 16

/* Helper function that forces register usage for arguments */
__attribute__((noinline, target("no-sse")))
static int helper_func(int a, int b, float c, double d, char e, 
                       int f, float g, double h, int i, float j) {
    volatile int sink;
    int result = a + b + (int)c + (int)d + e + f + (int)g + (int)h + i + (int)j;
    sink = result;
    return result;
}

/* Struct to force complex addressing */
struct DataNode {
    int values[SIZE];
    double matrix[4][4];
    struct DataNode *next;
    char metadata[32];
};

int main() {
    /* Multi-dimensional arrays with complex access patterns */
    int matrix[SIZE][SIZE];
    double arr3d[ITERS][SIZE/2][SIZE/2];
    char char_grid[SIZE][SIZE];
    float float_pool[SIZE * 2];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
            char_grid[i][j] = (i + j) % 256;
        }
    }
    
    for (int i = 0; i < SIZE * 2; i++) {
        float_pool[i] = i * 1.5f;
    }
    
    for (int i = 0; i < ITERS; i++) {
        for (int j = 0; j < SIZE/2; j++) {
            for (int k = 0; k < SIZE/2; k++) {
                arr3d[i][j][k] = i * 100.0 + j * 10.0 + k;
            }
        }
    }
    
    /* Struct with pointer chain */
    struct DataNode nodes[4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < SIZE; j++) {
            nodes[i].values[j] = i * 1000 + j;
        }
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                nodes[i].matrix[j][k] = i * 16.0 + j * 4.0 + k;
            }
        }
        if (i < 3) nodes[i].next = &nodes[i + 1];
        else nodes[i].next = &nodes[0];
        snprintf(nodes[i].metadata, 32, "node%d_data", i);
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, arr3d, char_grid, float_pool, nodes) \
                      map(tofrom: checksum)
    {
        /* Create massive register pressure with many local variables */
        register int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
        register float f0, f1, f2, f3, f4, f5, f6, f7;
        register double d0, d1, d2, d3, d4;
        register char c0, c1, c2, c3;
        volatile int vsink;
        volatile double vdsink;
        
        /* Initialize from mapped arrays with complex addressing */
        r0 = matrix[0][0];
        r1 = matrix[SIZE-1][SIZE-1];
        r2 = matrix[1][SIZE/2] + matrix[SIZE/2][1];
        
        /* Complex pointer arithmetic and struct access */
        struct DataNode *node_ptr = &nodes[0];
        r3 = node_ptr->values[(r0 + r1) % SIZE];
        r4 = node_ptr->next->next->values[(r2 * 3) % SIZE];
        
        /* Mixed type computations */
        f0 = float_pool[r0 % (SIZE * 2)];
        f1 = float_pool[r1 % (SIZE * 2)];
        d0 = arr3d[0][r0 % (SIZE/2)][r1 % (SIZE/2)];
        d1 = arr3d[1][r2 % (SIZE/2)][r3 % (SIZE/2)];
        
        c0 = char_grid[r0 % SIZE][r1 % SIZE];
        c1 = char_grid[r2 % SIZE][r3 % SIZE];
        
        /* Long computation chain keeping many variables live */
        for (int iter = 0; iter < ITERS; iter++) {
            /* Complex array indexing */
            int idx1 = (iter * 17 + r0) % SIZE;
            int idx2 = (iter * 23 + r1) % SIZE;
            int idx3 = (iter * 37 + r2) % SIZE;
            
            /* More local variables inside loop */
            int t0, t1, t2, t3, t4;
            float tf0, tf1;
            double td0, td1;
            
            /* Force address computations into registers */
            t0 = matrix[idx1][idx2] + matrix[idx2][idx3];
            t1 = matrix[idx3][idx1] * 2;
            
            /* Inline assembly with register constraints */
            asm volatile (
                "add %0, %1, %2\n\t"
                "sub %3, %4, %5"
                : "=r"(t2), "=r"(t3)
                : "r"(t0), "r"(t1), "r"(iter), "r"(r4)
                : "cc"
            );
            
            /* Mixed-type operations forcing mode conversions */
            tf0 = (float)t0 * f0 + (float)t1 * f1;
            td0 = (double)t2 * d0 + (double)t3 * d1;
            
            /* More inline assembly with specific constraints */
            int temp_int;
            float temp_float;
            asm volatile (
                "fmov %s0, %w1\n\t"
                : "=w"(temp_float)
                : "r"(t2)
            );
            
            asm volatile (
                "mov %0, %1\n\t"
                : "=r"(temp_int)
                : "w"(tf0)
            );
            
            /* Complex struct access with pointer chain */
            td1 = node_ptr->matrix[iter % 4][(t0 + t1) % 4];
            td1 += node_ptr->next->matrix[(t2 + t3) % 4][iter % 4];
            
            /* Force output reloads with volatile and pointer derefs */
            vsink = t0 + t1 + t2 + t3;
            vdsink = td0 + td1;
            
            /* Array store with computed index - forces out reload */
            char_grid[(idx1 + idx2) % SIZE][(idx2 + idx3) % SIZE] = 
                (char)((t0 + t1 + t2 + t3) % 256);
            
            /* Update register variables to keep them live */
            r5 = t0 + r5;
            r6 = t1 + r6;
            r7 = t2 + r7;
            r8 = t3 + r8;
            f2 = tf0 + f2;
            f3 = temp_float + f3;
            d2 = td0 + d2;
            d3 = td1 + d3;
            c2 = (char)(c0 + c1 + iter);
            c3 = (char)(c2 + t0 % 256);
            
            /* Function call forcing register arguments */
            if (iter % 4 == 0) {
                r9 = helper_func(r0, r1, f0, d0, c0,
                                 r2, f1, d1, r3, f2);
            }
            
            /* More arithmetic to chain variables */
            r0 = (r0 * 1103515245 + 12345) & 0x7fffffff;
            r1 = (r1 * 1664525 + 1013904223) & 0x7fffffff;
            f0 = f0 * 1.1f + tf0 * 0.9f;
            d0 = d0 * 1.01 + td0 * 0.99;
            c0 = (c0 + c1 + c2 + c3 + iter) % 256;
        }
        
        /* Final computation for checksum */
        checksum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                   (int)f0 + (int)f1 + (int)f2 + (int)f3 +
                   (int)d0 + (int)d1 + (int)d2 + (int)d3 +
                   c0 + c1 + c2 + c3;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

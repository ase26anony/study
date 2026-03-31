#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 128
#define ITERS 16

/* Helper function to force register usage */
__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, float f, double g, char h) {
    volatile int sink;
    sink = a + b - c * d + e;
    sink += (int)(f * g) + h;
    return sink;
}

/* Struct to force complex addressing */
struct DataNode {
    int data[SIZE];
    double values[SIZE];
    struct DataNode *next;
};

int main() {
    /* Multi-dimensional arrays for complex addressing */
    int matrix[SIZE][SIZE];
    double arr3d[SIZE][SIZE][SIZE];
    char char_arr[SIZE * SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
            for (int k = 0; k < SIZE; k++) {
                arr3d[i][j][k] = (i * 1.5 + j * 2.5 + k * 0.5);
            }
        }
    }
    for (int i = 0; i < SIZE * SIZE; i++) {
        char_arr[i] = (i % 128) - 64;
    }
    
    /* Struct with pointer chain */
    struct DataNode nodes[4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < SIZE; j++) {
            nodes[i].data[j] = i * 1000 + j;
            nodes[i].values[j] = i * 100.0 + j * 0.5;
        }
        if (i < 3) nodes[i].next = &nodes[i + 1];
        else nodes[i].next = &nodes[0];
    }
    
    int result = 0;
    
    #pragma omp target map(to: matrix[0:SIZE][0:SIZE], \
                                 arr3d[0:SIZE][0:SIZE][0:SIZE], \
                                 char_arr[0:SIZE*SIZE], \
                                 nodes[0:4]) \
                      map(tofrom: result)
    {
        /* Create register pressure with many local variables */
        register int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
        float f0, f1, f2, f3, f4, f5, f6, f7;
        double d0, d1, d2, d3, d4;
        char c0, c1, c2, c3;
        volatile int vsink;
        volatile double vdsink;
        
        /* Initialize from mapped arrays with complex addressing */
        r0 = matrix[0][0];
        r1 = matrix[SIZE-1][SIZE-1];
        f0 = (float)arr3d[0][0][0];
        d0 = arr3d[SIZE-1][SIZE-1][SIZE-1];
        c0 = char_arr[0];
        
        struct DataNode *current = &nodes[0];
        
        for (int iter = 0; iter < ITERS; iter++) {
            /* Complex array indexing forcing address reloads */
            int idx1 = (iter * 17 + r0) % SIZE;
            int idx2 = (iter * 23 + r1) % SIZE;
            int idx3 = (iter * 37 + idx1 * idx2) % SIZE;
            
            /* Force output reloads with pointer dereferences */
            int *ptr = &current->data[idx1];
            double *dptr = &current->values[idx2];
            
            /* Chain computations to keep variables live */
            r2 = matrix[idx1][idx2] + matrix[idx2][idx1];
            r3 = matrix[idx3][idx1] * matrix[idx1][idx3];
            
            /* Mixed-type operations forcing mode conversions */
            f1 = f0 + (float)r2 * 0.5f;
            f2 = (float)r3 * 0.25f - f1;
            
            /* Inline assembly with register constraints */
            asm volatile (
                "add %0, %1, %2\n\t"
                "sub %3, %4, %5"
                : "=r"(r4), "=r"(r5)
                : "r"(r2), "r"(r3), "r"(idx1), "r"(idx2)
                : "cc"
            );
            
            /* More register pressure */
            d1 = arr3d[idx1][idx2][idx3] + arr3d[idx3][idx2][idx1];
            d2 = d0 * 1.5 + d1 * 0.5;
            
            /* Force secondary reloads with mixed register classes */
            #ifdef __aarch64__
            /* Move between general and FP registers */
            asm volatile (
                "fmov %s0, %w1\n\t"
                "fmov %w2, %s3"
                : "=w"(f3), "=r"(r6)
                : "w"(f2), "r"(r4)
            );
            #else
            /* x86 version with xmm registers */
            asm volatile (
                "movd %0, %1\n\t"
                "movd %2, %3"
                : "=x"(f3), "=r"(r6)
                : "x"(f2), "r"(r4)
            );
            #endif
            
            /* Complex addressing with struct pointer chain */
            r7 = current->data[idx3] + current->next->data[idx1];
            d3 = current->values[idx2] - current->next->values[idx3];
            
            /* More inline assembly with memory constraints */
            asm volatile (
                "ldr %0, [%1]\n\t"
                "str %2, [%3]"
                : "=r"(r8)
                : "r"(ptr), "r"(r7), "r"(ptr)
                : "memory"
            );
            
            /* Force output reload to memory location */
            *ptr = r4 + r5 + r6;
            *dptr = d1 + d2 + d3;
            
            /* Volatile assignments forcing stores */
            vsink = r8;
            vdsink = d3;
            
            /* Function call with many register arguments */
            r9 = helper_func(r0, r1, r2, r3, r4, f0, d0, c0);
            
            /* Update variables to keep them live */
            r0 = (r0 + r9) % SIZE;
            r1 = (r1 + r8) % SIZE;
            f0 = f0 * 0.9f + f3 * 0.1f;
            d0 = d0 * 0.9 + d3 * 0.1;
            c0 = (c0 + char_arr[idx3]) % 128;
            
            /* Rotate through struct nodes */
            current = current->next;
            
            /* More computations to increase pressure */
            r10 = r0 * r1 - r2 * r3 + r4 * r5 - r6 * r7 + r8 * r9;
            f4 = f0 + f1 + f2 + f3;
            f5 = f4 * 2.0f - f0;
            f6 = f5 / 3.0f + f1;
            f7 = f6 * f2 - f3;
            
            d4 = d0 + d1 + d2 + d3;
            c1 = (c0 + r10) & 0xFF;
            c2 = (c1 + idx1) & 0xFF;
            c3 = (c2 + idx2) & 0xFF;
            
            /* Use all variables in final computation */
            result += r10 + (int)f7 + (int)d4 + c1 + c2 + c3;
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}

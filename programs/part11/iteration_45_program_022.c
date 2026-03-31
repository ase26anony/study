#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define ITERS 100

/* Struct to create pointer chains */
struct DataNode {
    int data[SIZE];
    struct DataNode *next;
    double values[SIZE];
    char metadata[SIZE];
};

/* Non-inline helper to force register usage for arguments */
__attribute__((noinline)) 
int compute_checksum(int a, int b, int c, float d, double e, char f, int g, float h) {
    volatile int result;
    /* Force register pressure with inline asm */
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %7"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(g), "r"(f), "r"(h), "r"(g)
        : "cc"
    );
    return result + (int)d + (int)e + f;
}

int main() {
    /* Multi-dimensional arrays with complex access patterns */
    int matrix[SIZE][SIZE];
    double tensor[SIZE][SIZE][2];
    char buffer[SIZE * SIZE];
    volatile int sink; /* For forcing output reloads */
    
    struct DataNode node1, node2;
    struct DataNode *ptr1 = &node1;
    struct DataNode *ptr2 = &node2;
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
            tensor[i][j][0] = i * 0.5 + j * 0.25;
            tensor[i][j][1] = i * 0.3 - j * 0.7;
        }
        buffer[i] = i % 256;
        node1.data[i] = i * 2;
        node1.values[i] = i * 1.5;
        node1.metadata[i] = i;
        node2.data[i] = i * 3;
        node2.values[i] = i * 2.5;
        node2.metadata[i] = i + 64;
    }
    node1.next = &node2;
    node2.next = &node1;
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, tensor, buffer, node1, node2) \
                       map(tofrom: checksum)
    {
        /* Declare many local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
        float f1, f2, f3, f4, f5, f6, f7, f8;
        double d1, d2, d3, d4, d5;
        char c1, c2, c3, c4, c5;
        long l1, l2, l3;
        
        /* Initialize from mapped data with complex addressing */
        v1 = matrix[0][0];
        v2 = matrix[SIZE-1][SIZE-1];
        f1 = tensor[0][0][0];
        d1 = tensor[SIZE-1][SIZE-1][1];
        c1 = buffer[0];
        
        /* Complex loop with register pressure */
        for (int iter = 0; iter < ITERS; iter++) {
            int i = iter % SIZE;
            int j = (iter * 7) % SIZE;
            int k = (iter * 13) % SIZE;
            
            /* Complex array indexing forcing address reloads */
            int idx1 = (i * SIZE + j) % SIZE;
            int idx2 = (j * SIZE + k) % SIZE;
            int idx3 = (k * SIZE + i) % SIZE;
            
            /* Chain computations keeping many variables live */
            v3 = matrix[idx1][idx2] + matrix[idx2][idx3];
            v4 = matrix[idx3][idx1] * v3;
            
            /* Pointer chain access forcing base address computation */
            v5 = ptr1->data[idx1] + ptr2->data[idx2];
            v6 = ptr1->next->data[idx3] - ptr2->next->data[idx1];
            
            /* Mixed type computations forcing mode conversions */
            f2 = tensor[i][j][0] + tensor[j][k][1];
            d2 = tensor[k][i][0] * f2 + d1;
            
            /* Inline assembly with register constraints */
            asm volatile (
                "add %0, %1, %2\n\t"
                "mul %0, %0, %3"
                : "=r"(v7)
                : "r"(v3), "r"(v4), "r"(v5)
                : "cc"
            );
            
            /* Another asm with different constraints */
            asm volatile (
                "fadd %s0, %s1, %s2\n\t"
                "fmul %s0, %s0, %s3"
                : "=w"(f3)
                : "w"(f1), "w"(f2), "w"(f2)
            );
            
            /* Force output reload to memory with volatile */
            sink = v7 + (int)f3;
            
            /* Complex assignment to array element (output reload) */
            buffer[(i + j + k) % SIZE] = v7 & 0xFF;
            
            /* More variable chaining */
            v8 = v4 + v5;
            v9 = v6 * v7;
            v10 = v8 - v9;
            f4 = f2 * f3;
            d3 = d1 + d2;
            
            /* Secondary reload trigger: move between register classes */
            int ival = v10;
            float fval;
            asm volatile (
                "fmov %s0, %w1"
                : "=w"(fval)
                : "r"(ival)
            );
            
            /* Use builtin that may require specific registers */
            v11 = __builtin_popcount(v8);
            v12 = __builtin_clz(v9);
            
            /* More mixed operations */
            c2 = buffer[idx1];
            c3 = buffer[idx2];
            v13 = v11 + c2 + c3;
            f5 = fval + c2 * 0.5;
            
            /* Struct member assignment with computed pointer */
            ptr1->metadata[idx1] = c2 + c3;
            
            /* Call helper function forcing register arguments */
            v14 = compute_checksum(v1, v2, v3, f1, d1, c1, v4, f2);
            
            /* Final chain */
            v15 = v13 + v14 + (int)f5;
            checksum += v15;
            
            /* Rotate values to keep them all live */
            v1 = v2; v2 = v3; v3 = v4; v4 = v5;
            v5 = v6; v6 = v7; v7 = v8; v8 = v9;
            v9 = v10; v10 = v11; v11 = v12; v12 = v13;
            v13 = v14; v14 = v15;
            f1 = f2; f2 = f3; f3 = f4; f4 = f5;
            d1 = d2; d2 = d3;
            c1 = c2; c2 = c3;
        }
        
        /* Additional pressure with many simultaneous live variables */
        l1 = (long)v1 * v2 * v3 * v4;
        l2 = (long)v5 * v6 * v7 * v8;
        l3 = l1 + l2 + v9 + v10 + v11 + v12 + v13 + v14;
        
        /* Final output to volatile */
        sink = l3 + (int)d1 + (int)f1 + c1;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

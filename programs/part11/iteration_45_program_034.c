#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 128
#define NUM_VARS 25

/* Helper function to force register usage for arguments */
__attribute__((noinline))
static int compute_checksum(int a, int b, int c, float d, double e, 
                           char f, int g, float h, double i, int j) {
    volatile int result;
    /* Force register pressure with mixed operations */
    result = a + b * c + (int)d + (int)e + f + g + (int)h + (int)i + j;
    
    /* Inline asm with register constraints */
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(result), "r"(a));
    return result;
}

/* Struct to create pointer chains */
struct DataNode {
    int data[SIZE];
    float fdata[SIZE];
    struct DataNode *next;
};

int main() {
    /* Multi-dimensional arrays with complex access patterns */
    int matrix[SIZE][SIZE];
    double dmatrix[SIZE][SIZE];
    float farr[SIZE][SIZE][2];
    char carr[SIZE][SIZE];
    
    /* Struct with pointer chain */
    struct DataNode nodes[4];
    struct DataNode *ptr_chain = &nodes[0];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
            dmatrix[i][j] = (i * 0.5) + (j * 0.3);
            farr[i][j][0] = i * 1.1f;
            farr[i][j][1] = j * 2.2f;
            carr[i][j] = (i + j) % 256;
        }
    }
    
    /* Initialize linked structure */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < SIZE; j++) {
            nodes[i].data[j] = i * 1000 + j;
            nodes[i].fdata[j] = i * 100.0f + j;
        }
        nodes[i].next = (i < 3) ? &nodes[i+1] : NULL;
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, dmatrix, farr, carr, nodes) \
                       map(tofrom: checksum)
    {
        /* Declare many local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        float f1, f2, f3, f4, f5, f6, f7, f8;
        double d1, d2, d3, d4, d5, d6;
        char c1, c2, c3, c4;
        volatile int sink;  /* For forcing output reloads */
        volatile double dsink;
        
        /* Initialize from mapped arrays with complex indexing */
        v1 = matrix[0][0];
        v2 = matrix[1][SIZE-1];
        f1 = farr[0][0][0];
        f2 = farr[1][1][1];
        d1 = dmatrix[0][0];
        d2 = dmatrix[SIZE-1][SIZE-1];
        c1 = carr[0][0];
        c2 = carr[SIZE/2][SIZE/2];
        
        /* Pointer chain traversal forcing address computation */
        struct DataNode *current = ptr_chain;
        int offset = 0;
        
        /* Complex nested loops creating many live ranges */
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                /* Complex array indexing forcing address reloads */
                int idx1 = (i * 17 + j * 13) % SIZE;
                int idx2 = (i * 23 + j * 7) % SIZE;
                int idx3 = (i * 11 + j * 19) % SIZE;
                
                /* Mixed-type computations */
                v3 = matrix[idx1][idx2] + carr[idx2][idx1];
                f3 = farr[idx1][idx2][0] * 2.0f + farr[idx2][idx1][1];
                d3 = dmatrix[idx1][idx2] * 1.5 + dmatrix[idx2][idx1];
                
                /* Chain computations keeping many variables live */
                v4 = v1 + v2 * v3 - i + j;
                v5 = v2 + v3 * v4 / (j + 1);
                v6 = v3 + v4 * v5 % (i + 1);
                v7 = v4 + v5 * v6 & 0xFF;
                v8 = v5 + v6 * v7 | 0x7F;
                
                f4 = f1 + f2 * f3 / (j + 1.0f);
                f5 = f2 + f3 * f4 - i * 0.5f;
                f6 = f3 + f4 * f5 * 0.3f;
                
                d4 = d1 + d2 * d3 / (i + 1.0);
                d5 = d2 + d3 * d4 - j * 0.7;
                d6 = d3 + d4 * d5 * 1.1;
                
                c3 = c1 + c2 * (i % 256);
                c4 = c2 + c3 * (j % 256);
                
                /* Inline assembly with register constraints */
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "mul %3, %4, %5"
                    : "=r"(v9), "=r"(v10)
                    : "r"(v6), "r"(v7), "r"(v8), "0"(v9)
                    : "cc"
                );
                
                /* Force output reloads with volatile stores */
                sink = v9 + v10 + (int)f4 + (int)d4 + c3;
                
                /* More inline asm with mixed register classes */
                /* Simulating move between register classes */
                int ival = v7 + v8;
                float fval;
                asm volatile (
                    "fmov %s0, %w1"
                    : "=w"(fval)
                    : "r"(ival)
                );
                dsink = fval * 2.0;
                
                /* Complex pointer-based access forcing address reloads */
                if (current) {
                    /* Multi-step pointer dereference */
                    int *data_ptr = current->data;
                    float *fdata_ptr = current->fdata;
                    
                    /* Complex indexing with pointer arithmetic */
                    int data_val = data_ptr[(i * 3 + j * 5) % SIZE];
                    float fdata_val = fdata_ptr[(i * 7 + j * 11) % SIZE];
                    
                    /* Force reload with inline asm */
                    asm volatile (
                        "add %0, %1, %2"
                        : "=r"(data_val)
                        : "r"(data_val), "r"(v5)
                    );
                    
                    sink = data_val + (int)fdata_val;
                    
                    /* Move to next node */
                    if (current->next && (i + j) % 8 == 0) {
                        current = current->next;
                    }
                }
                
                /* Function call forcing argument passing in registers */
                if ((i + j) % 16 == 0) {
                    int call_result = compute_checksum(
                        v4, v5, v6, f4, d4, c3, v7, f5, d5, v8
                    );
                    sink = call_result;
                }
                
                /* Update checksum with mixed operations */
                checksum += v9 + v10 + (int)f6 + (int)d6 + c4;
                checksum &= 0xFFFFFF;  /* Prevent overflow */
            }
        }
        
        /* Final complex expression forcing last reloads */
        int final_val = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                       (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 +
                       (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6 +
                       c1 + c2 + c3 + c4;
        
        /* Force one more output reload */
        volatile int final_sink;
        final_sink = final_val;
        checksum += final_sink;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 128
#define NUM_VARS 25

/* Struct to force complex addressing */
struct DataNode {
    int values[4];
    double fp_data[3];
    struct DataNode *next;
};

/* Non-inline function to force register usage for arguments */
__attribute__((noinline)) 
int compute_checksum(int a, int b, int c, float d, double e, char f) {
    volatile int result;
    /* Force register pressure with inline asm */
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    result += c + (int)d + (int)e + f;
    return result;
}

int main() {
    /* Multi-dimensional arrays with complex access patterns */
    int matrix[SIZE][SIZE];
    double arr3d[32][32][32];
    float float_arr[SIZE * 2];
    char char_arr[SIZE * 4];
    
    /* Struct with pointer chain */
    struct DataNode nodes[SIZE];
    struct DataNode *ptr_chain[SIZE / 2];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 32; k++) {
                arr3d[i % 32][j][k] = (i + j + k) * 1.5;
            }
        }
        float_arr[i] = i * 0.75f;
        char_arr[i] = (i % 128) - 64;
        
        nodes[i].values[0] = i;
        nodes[i].values[1] = i * 2;
        nodes[i].values[2] = i * 3;
        nodes[i].values[3] = i * 4;
        nodes[i].fp_data[0] = i * 1.1;
        nodes[i].fp_data[1] = i * 2.2;
        nodes[i].fp_data[2] = i * 3.3;
        nodes[i].next = &nodes[(i + 1) % SIZE];
        
        if (i < SIZE / 2) {
            ptr_chain[i] = &nodes[i * 2];
        }
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix[0:SIZE][0:SIZE], \
                                 arr3d[0:32][0:32][0:32], \
                                 float_arr[0:SIZE*2], \
                                 char_arr[0:SIZE*4], \
                                 nodes[0:SIZE], \
                                 ptr_chain[0:SIZE/2]) \
                     map(from: checksum)
    {
        /* Declare many local variables to create register pressure */
        register int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
        register float f0, f1, f2, f3, f4, f5;
        register double d0, d1, d2, d3, d4;
        register char c0, c1, c2, c3;
        volatile int sink;  /* For forcing output reloads */
        volatile double dsink;
        
        /* Initialize from mapped arrays with complex addressing */
        v0 = matrix[0][0];
        v1 = matrix[SIZE-1][SIZE-1];
        v2 = matrix[32][64] + matrix[64][32];
        
        /* Complex array indexing forcing address computation */
        int stride = 17;
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                /* Force address reloads with complex indices */
                int idx = (i * stride + j * 3) % SIZE;
                int idx2 = (j * stride + i * 7) % SIZE;
                
                /* Chain computations keeping many variables live */
                v3 = matrix[idx][idx2];
                v4 = matrix[idx2][idx];
                
                /* Mixed type operations forcing mode conversions */
                f0 = float_arr[idx] + float_arr[idx2];
                d0 = arr3d[i % 32][j % 32][(i + j) % 32];
                
                /* Pointer chain access forcing base address computation */
                struct DataNode *node_ptr = ptr_chain[i % (SIZE / 2)];
                v5 = node_ptr->values[(i + j) % 4];
                d1 = node_ptr->fp_data[(i + j) % 3];
                
                /* Inline assembly with register constraints */
                asm volatile ("add %0, %1, %2" : "=r"(v6) : "r"(v3), "r"(v4));
                asm volatile ("mul %0, %1, %2" : "=r"(v7) : "r"(v5), "r"(i));
                
                /* Force output reload to memory with volatile */
                sink = v6 + v7;
                
                /* More complex addressing with struct pointer chain */
                v8 = node_ptr->next->values[(i * j) % 4];
                v9 = node_ptr->next->next->values[(i + j * 2) % 4];
                
                /* Inline asm forcing specific register class usage */
                /* This may trigger secondary reloads on some architectures */
                int temp_int = v8 * 3;
                double temp_double;
                /* Attempt to move between register classes */
                asm volatile ("/* potential reg class move */" : "=r"(temp_int) : "0"(temp_int));
                
                /* Mixed-type expression with char */
                c0 = char_arr[(i * 4 + j) % (SIZE * 4)];
                c1 = char_arr[(j * 4 + i) % (SIZE * 4)];
                v2 += c0 - c1;
                
                /* Force floating-point operations */
                f1 = f0 * 2.0f + (float)v5;
                f2 = f1 / (float)(v6 + 1);
                
                /* More inline asm with memory constraints */
                asm volatile ("ldr %0, [%1]" : "=r"(v4) : "r"(&matrix[idx][idx2]));
                
                /* Complex expression for array store (output reload) */
                int store_idx = (i * 19 + j * 13) % SIZE;
                matrix[store_idx][(store_idx * 2) % SIZE] = v4 + v6 + v8;
                
                /* Volatile store forcing output reload */
                dsink = d0 + d1 + (double)f2;
                
                /* Chain variables to keep them all live */
                v0 += v1; v1 += v2; v2 += v3; v3 += v4;
                v4 += v5; v5 += v6; v6 += v7; v7 += v8;
                v8 += v9; v9 += v0;
                
                f0 += f1; f1 += f2; f2 += f3; f3 += f4;
                d0 += d1; d1 += d2; d2 += d3;
                
                /* Function call forcing register usage for arguments */
                if ((i * j) % 7 == 0) {
                    int func_result = compute_checksum(v0, v1, v2, f0, d0, c0);
                    checksum += func_result;
                }
            }
        }
        
        /* Final complex computation using all variables */
        int final_result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        final_result += (int)(f0 + f1 + f2 + f3 + f4 + f5);
        final_result += (int)(d0 + d1 + d2 + d3 + d4);
        final_result += c0 + c1 + c2 + c3;
        
        checksum += final_result;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

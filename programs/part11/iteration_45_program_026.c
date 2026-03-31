#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define NUM_VARS 25

typedef struct {
    int data[16];
    double values[8];
    char *ptr;
} ComplexStruct;

__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, float f, double g) {
    volatile int result;
    // Force register usage and potential reloads
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %1, %2" : "+r"(result) : "r"(c), "r"(d));
    return result + (int)(f * g);
}

int main() {
    // Initialize complex data structures
    int matrix[SIZE][SIZE];
    double arr3d[SIZE][SIZE][SIZE];
    ComplexStruct structs[SIZE];
    volatile int sink; // For forcing output reloads
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
        for (int j = 0; j < SIZE; j++) {
            for (int k = 0; k < SIZE; k++) {
                arr3d[i][j][k] = (i * j * k) * 0.1;
            }
        }
        for (int j = 0; j < 16; j++) {
            structs[i].data[j] = i + j;
        }
        for (int j = 0; j < 8; j++) {
            structs[i].values[j] = (i + j) * 0.5;
        }
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, arr3d, structs) map(from: checksum)
    {
        // Declare many local variables to create register pressure
        register int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
        float f0, f1, f2, f3, f4, f5;
        double d0, d1, d2, d3, d4;
        char c0, c1, c2, c3;
        int * volatile ptr_var; // volatile pointer for address reloads
        
        // Initialize from mapped arrays with complex addressing
        v0 = matrix[0][0];
        v1 = matrix[SIZE-1][SIZE-1];
        v2 = structs[0].data[0];
        v3 = structs[SIZE/2].data[7];
        d0 = arr3d[0][0][0];
        d1 = arr3d[SIZE-1][SIZE-1][SIZE-1];
        f0 = (float)d0;
        f1 = (float)d1;
        
        // Complex nested loop creating many live ranges
        for (int i = 1; i < SIZE/4; i++) {
            for (int j = 1; j < SIZE/4; j++) {
                // Complex array indexing forcing address computation
                int idx1 = (i * SIZE + j * 3) % SIZE;
                int idx2 = (j * SIZE + i * 7) % SIZE;
                int idx3 = (i * j) % 16;
                
                // Chain computations keeping many variables live
                v4 = matrix[idx1][idx2];
                v5 = structs[i].data[idx3];
                v6 = structs[j].data[(idx1 + idx2) % 16];
                
                // Mixed type operations forcing mode conversions
                f2 = (float)v4 * 0.5f;
                f3 = (float)v5 * 1.5f;
                d2 = (double)v6 * 0.25;
                
                // Inline assembly with register constraints
                asm volatile ("add %0, %1, %2" : "=r"(v7) : "r"(v4), "r"(v5));
                asm volatile ("mul %0, %1, %2" : "+r"(v7) : "r"(v6), "r"(i));
                
                // More arithmetic creating interference
                v8 = v7 * j;
                v9 = v8 + v4 - v5;
                f4 = f2 + f3;
                d3 = d2 * d0 + d1;
                
                // Force output reloads with volatile and pointer stores
                sink = v9;  // Volatile store
                
                // Complex pointer arithmetic and dereference
                ptr_var = &matrix[i][j];
                *ptr_var = v9;  // Output reload through pointer
                
                // Multi-dimensional array store with complex index
                int store_idx = (i * 3 + j * 7) % SIZE;
                matrix[store_idx][(i + j) % SIZE] = v8;
                
                // Mixed type inline assembly (potential secondary reloads)
                int ival = v9;
                float fval;
                // Attempt to move between register classes
                asm volatile ("fcvt %s0, %w1" : "=w"(fval) : "r"(ival));
                
                // Use all variables to keep them live
                c0 = (char)(v0 & 0xFF);
                c1 = (char)(v1 & 0xFF);
                c2 = (char)(v2 & 0xFF);
                c3 = (char)(v3 & 0xFF);
                
                f5 = f0 + f1 + f4;
                d4 = d3 + d2 + (c0 + c1 + c2 + c3) * 0.01;
                
                // Function call with many register arguments
                int func_result = helper_func(v4, v5, v6, v7, v8, f5, d4);
                
                // Update checksum
                checksum += func_result + v9 + (int)fval;
                
                // Rotate values to extend live ranges
                v0 = v1; v1 = v2; v2 = v3; v3 = v4;
                v4 = v5; v5 = v6; v6 = v7; v7 = v8;
                f0 = f1; f1 = f2; f2 = f3; f3 = f4;
                d0 = d1; d1 = d2; d2 = d3; d3 = d4;
            }
        }
        
        // Additional pressure with unrolled operations
        for (int k = 0; k < 8; k++) {
            // Multiple inline asm operations in sequence
            int tmp1, tmp2, tmp3;
            asm volatile ("mov %0, %1" : "=r"(tmp1) : "r"(k));
            asm volatile ("lsl %0, %1, #2" : "=r"(tmp2) : "r"(tmp1));
            asm volatile ("add %0, %1, %2" : "=r"(tmp3) : "r"(tmp2), "r"(checksum));
            
            // Access struct through pointer chain
            ComplexStruct *sptr = &structs[k];
            sptr->data[k % 16] = tmp3;
            
            // Volatile double store
            volatile double *dptr = &sptr->values[k % 8];
            *dptr = d0 + k * 0.1;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

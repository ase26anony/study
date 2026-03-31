#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define SIZE 128
#define DIM 16

typedef struct {
    int data[SIZE];
    double values[SIZE];
    char *ptr;
    int offset;
} DataStruct;

typedef struct {
    DataStruct *ds;
    int matrix[DIM][DIM];
    float coeffs[SIZE];
} Container;

// Force noinline to create register pressure for arguments
__attribute__((noinline)) 
int compute_helper(int a, int b, int c, int d, int e, float f, double g) {
    volatile int result;
    // Force register usage with inline asm
    asm volatile (
        "add %w0, %w1, %w2\n\t"
        "add %w0, %w0, %w3\n\t"
        "add %w0, %w0, %w4"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d)
        : "cc"
    );
    
    // Mixed type operations
    double temp = g + (double)f + (double)e;
    asm volatile ("" : "+r"(result), "+r"(temp) : : "memory");
    
    return result + (int)temp;
}

int main() {
    // Initialize complex data structures
    Container *cont = (Container *)malloc(sizeof(Container));
    cont->ds = (DataStruct *)malloc(sizeof(DataStruct));
    cont->ds->ptr = (char *)malloc(SIZE * sizeof(char));
    
    // Initialize arrays with pattern
    for (int i = 0; i < SIZE; i++) {
        cont->ds->data[i] = i * 3;
        cont->ds->values[i] = i * 1.5;
        cont->coeffs[i] = i * 0.25f;
        cont->ds->ptr[i] = (i % 26) + 'a';
    }
    
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            cont->matrix[i][j] = i * DIM + j;
        }
    }
    
    int result = 0;
    
    #pragma omp target map(tofrom: result) \
                      map(to: cont[0:1], cont->ds[0:1], cont->ds->ptr[0:SIZE], \
                          cont->matrix[0:DIM][0:DIM], cont->coeffs[0:SIZE], \
                          cont->ds->data[0:SIZE], cont->ds->values[0:SIZE])
    {
        // Create many local variables to consume registers
        int v1 = cont->ds->data[0];
        int v2 = cont->ds->data[1];
        int v3 = cont->matrix[0][0];
        int v4 = cont->matrix[1][1];
        int v5 = cont->ds->data[2];
        int v6 = cont->matrix[2][2];
        int v7 = cont->ds->data[3];
        int v8 = cont->matrix[3][3];
        int v9 = cont->ds->data[4];
        int v10 = cont->matrix[4][4];
        
        float f1 = cont->coeffs[0];
        float f2 = cont->coeffs[1];
        float f3 = cont->coeffs[2];
        float f4 = cont->coeffs[3];
        float f5 = cont->coeffs[4];
        
        double d1 = cont->ds->values[0];
        double d2 = cont->ds->values[1];
        double d3 = cont->ds->values[2];
        double d4 = cont->ds->values[3];
        double d5 = cont->ds->values[4];
        
        char c1 = cont->ds->ptr[0];
        char c2 = cont->ds->ptr[1];
        char c3 = cont->ds->ptr[2];
        char c4 = cont->ds->ptr[3];
        char c5 = cont->ds->ptr[4];
        
        volatile int sink1, sink2, sink3;
        volatile float fsink;
        volatile double dsink;
        
        // Complex nested loops with addressing that requires reloads
        for (int i = 0; i < DIM/2; i++) {
            for (int j = 0; j < DIM/2; j++) {
                // Complex array indexing - forces address computation in registers
                int idx1 = (i * DIM + j * 3) % SIZE;
                int idx2 = (j * DIM + i * 7) % SIZE;
                int idx3 = ((i + j) * 11) % SIZE;
                
                // Chain computations keeping many variables live
                v1 = cont->ds->data[idx1] + v2;
                v2 = cont->matrix[i][j] * v3;
                v3 = v4 + cont->ds->data[idx2];
                v4 = v5 - cont->matrix[j][i];
                
                // Mixed type operations forcing conversions
                f1 = (float)v1 * f2 + f3;
                f2 = f4 / (float)(v2 + 1);
                d1 = (double)f1 + d2 * 1.5;
                d2 = d3 - (double)v3 * 0.25;
                
                // Inline assembly with register constraints
                // Force specific register usage
                asm volatile (
                    "add %w0, %w1, %w2\n\t"
                    "mul %w0, %w0, %w3"
                    : "=r"(v5)
                    : "r"(v1), "r"(v2), "r"(v3)
                    : "cc"
                );
                
                // Another asm with different constraints
                asm volatile (
                    "fadd %s0, %s1, %s2"
                    : "=w"(f3)
                    : "w"(f1), "w"(f2)
                );
                
                // Force secondary reloads: move between register classes
                // This may require secondary reloads on some architectures
                int temp_int = v5 + v6;
                float temp_float;
                asm volatile (
                    "scvtf %s0, %w1"
                    : "=w"(temp_float)
                    : "r"(temp_int)
                );
                f4 = temp_float + f5;
                
                // Complex pointer chain access - forces address reload
                char *ptr = cont->ds->ptr;
                int offset = (i * 17 + j * 13) % SIZE;
                c1 = ptr[offset] + c2;
                c2 = ptr[(offset + 5) % SIZE] - c3;
                
                // Volatile stores - force output reloads
                sink1 = v1 + v2 + v3;
                sink2 = v4 * v5 - v6;
                fsink = f1 * f2 + f3;
                dsink = d1 / (d2 + 1.0);
                
                // Assignment to computed array element - output reload
                cont->matrix[i][j] = v1 + v2;
                
                // Complex addressing with struct member
                cont->ds->data[(i * 3 + j * 7) % SIZE] = v3 + v4;
                
                // Call helper function - creates register pressure for args
                v6 = compute_helper(v1, v2, v3, v4, v5, f1, d1);
                
                // Use builtin that may need specific registers
                v7 = __builtin_popcount(v6) + v8;
                
                // More mixed operations
                v8 = (int)((double)v7 * d2) + (int)f4;
                v9 = v8 + c1 + c2;
                v10 = v9 * (c3 - c4);
                
                // Chain continues...
                f5 = (float)v10 * 0.1f + f4;
                d3 = d4 + (double)v9 * 0.01;
                d4 = d5 - d3 * 0.5;
                d5 = (double)(c5 * 2) + d1;
                
                c3 = c4 + (char)(v1 % 26);
                c4 = c5 - (char)(v2 % 26);
                c5 = ptr[(offset + 11) % SIZE];
            }
        }
        
        // Final computation using all live variables
        result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                 + (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5
                 + (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5
                 + c1 + c2 + c3 + c4 + c5;
    }
    
    printf("Result: %d\n", result);
    
    // Cleanup
    free(cont->ds->ptr);
    free(cont->ds);
    free(cont);
    
    return 0;
}

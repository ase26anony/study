#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define SIZE 128
#define VARS 25

typedef struct {
    int data[SIZE];
    double values[SIZE];
    char *ptr;
} DataStruct;

__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, float f, double g) {
    volatile int result;
    // Force register usage with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %1, %2" : "+r"(result) : "r"(c), "r"(d));
    return result + (int)(f * g) + e;
}

int main() {
    int matrix[SIZE][SIZE];
    double arr3d[SIZE][SIZE][SIZE];
    DataStruct ds1, ds2;
    int *ptr_array[SIZE];
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
        ds1.data[i] = i * 2;
        ds1.values[i] = i * 1.5;
        ptr_array[i] = &ds1.data[i];
    }
    ds1.ptr = (char*)&ds1.data[0];
    memcpy(&ds2, &ds1, sizeof(DataStruct));
    
    int checksum = 0;
    
    #pragma omp target map(to: matrix, arr3d, ds1, ptr_array) \
                       map(tofrom: ds2) map(from: checksum)
    {
        // Declare many local variables to create register pressure
        register int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
        float f0, f1, f2, f3, f4, f5;
        double d0, d1, d2, d3, d4;
        char c0, c1, c2, c3;
        volatile int sink;  // For forcing output reloads
        
        // Initialize from mapped data with complex addressing
        v0 = matrix[0][0];
        v1 = matrix[1][SIZE-1];
        v2 = ds1.data[(v0 + v1) % SIZE];
        v3 = *ptr_array[(v2 * 3) % SIZE];
        
        // Complex pointer chain
        char *ptr1 = ds1.ptr + v0 * sizeof(int);
        char *ptr2 = ds2.ptr + v1 * sizeof(int);
        
        // Multi-dimensional array access with complex index
        #pragma omp parallel for reduction(+:checksum)
        for (int i = 0; i < SIZE/2; i++) {
            // Local variables inside loop - more pressure
            int l0, l1, l2, l3, l4, l5;
            float lf0, lf1, lf2;
            double ld0, ld1;
            
            // Complex index calculations forcing address reloads
            int idx1 = (i * 17 + v0) % SIZE;
            int idx2 = (i * 23 + v1) % SIZE;
            int idx3 = (i * 31 + v2) % SIZE;
            
            // Chain computations with mixed types
            l0 = matrix[idx1][idx2];
            l1 = matrix[idx2][idx3];
            l2 = matrix[idx3][idx1];
            
            // Force address computation into register
            int *base_ptr = &matrix[0][0];
            l3 = base_ptr[idx1 * SIZE + idx2];
            l4 = base_ptr[idx2 * SIZE + idx3];
            
            // Mixed type operations forcing mode conversions
            f0 = (float)l0 * 1.5f;
            f1 = (float)l1 * 2.5f;
            d0 = (double)l2 * 3.14159;
            d1 = (double)l3 * 2.71828;
            
            // Inline assembly with register constraints
            asm volatile ("add %0, %1, %2" : "=r"(l5) : "r"(l0), "r"(l1));
            asm volatile ("mul %0, %1, %2" : "+r"(l5) : "r"(l2), "r"(l3));
            
            // More assembly with different constraints
            int temp;
            asm volatile ("mov %0, %1" : "=r"(temp) : "r"(l4));
            asm volatile ("and %0, %1, %2" : "+r"(temp) : "r"(l5), "i"(0xFF));
            
            // Force output reload with volatile store
            sink = temp;
            
            // Complex struct member access
            lf0 = (float)ds1.values[idx1] + (float)ds2.values[idx2];
            ld0 = ds1.values[idx3] * ds2.values[idx1];
            
            // Pointer arithmetic forcing address reloads
            int *dptr = &ds2.data[0];
            dptr += idx1 * 2 - idx2;
            l0 = *dptr;
            
            dptr += idx3 - idx1;
            l1 = *dptr;
            
            // 3D array access - complex address computation
            double *arr_ptr = &arr3d[0][0][0];
            int offset = (idx1 * SIZE * SIZE + idx2 * SIZE + idx3) % (SIZE*SIZE*SIZE);
            ld1 = arr_ptr[offset];
            
            // Call helper function - forces calling convention handling
            l2 = helper_func(l0, l1, l3, l4, l5, lf0, ld0);
            
            // Assignment to array element with computed index
            ds2.data[(i + l2) % SIZE] = l2;
            
            // More mixed operations
            c0 = (char)(l0 & 0xFF);
            c1 = (char)(l1 & 0xFF);
            c2 = (char)(l2 & 0xFF);
            
            // Character operations forcing byte register usage
            c3 = c0 + c1 - c2;
            sink = c3;
            
            checksum += l0 + l1 + l2 + (int)lf0 + (int)ld0 + (int)ld1;
        }
        
        // Additional computation outside loop
        v4 = v0 * v1 - v2 + v3;
        v5 = v1 * v2 / (v3 ? v3 : 1);
        v6 = v2 ^ v3 ^ v0;
        v7 = v3 | v0 | v1;
        
        // Chain computations
        for (int i = 0; i < 8; i++) {
            v8 = v4 + v5;
            v9 = v6 * v7;
            v4 = v5 - v6;
            v5 = v7 / (v8 ? v8 : 1);
            v6 = v8 ^ v9;
            v7 = v4 | v5;
            
            // Force register spilling with many live variables
            f2 = (float)v8 * 0.5f;
            f3 = (float)v9 * 1.5f;
            d2 = (double)v4 * 3.14;
            d3 = (double)v5 * 2.71;
            
            // Use all variables in computation
            f4 = f0 + f1 + f2 + f3;
            d4 = d0 + d1 + d2 + d3;
            
            // Inline asm using floating point registers
            asm volatile ("fadd %s0, %s1, %s2" : "=w"(f5) : "w"(f2), "w"(f3));
            
            // Mixed register class usage
            int ifloat;
            asm volatile ("fmov %w0, %s1" : "=r"(ifloat) : "w"(f5));
            
            sink = ifloat + (int)d4;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

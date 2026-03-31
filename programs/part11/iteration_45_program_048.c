#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define ITERS 100

typedef struct {
    int data[16];
    double matrix[4][4];
    char *ptr;
} ComplexStruct;

__attribute__((noinline))
int helper_func(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = 0;
    // Force register usage with inline asm
    asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    asm volatile ("mul %0, %1, %2" : "+r"(result) : "r"(c), "r"(d));
    asm volatile ("sub %0, %1, %2" : "+r"(result) : "r"(e), "r"(f));
    asm volatile ("and %0, %1, %2" : "+r"(result) : "r"(g), "r"(h));
    return result;
}

int main() {
    // Initialize complex data structures
    int arr1[SIZE][SIZE];
    double arr2[SIZE][SIZE];
    ComplexStruct cs[8];
    char buffer[SIZE * SIZE];
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            arr1[i][j] = i * SIZE + j;
            arr2[i][j] = (double)(i * j) / 100.0;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            cs[i].data[j] = i * 100 + j;
        }
        for (int k = 0; k < 4; k++) {
            for (int l = 0; l < 4; l++) {
                cs[i].matrix[k][l] = (double)(i + k + l) / 2.0;
            }
        }
        cs[i].ptr = buffer + i * 64;
    }
    
    int checksum = 0;
    
    #pragma omp target map(to: arr1, arr2, cs, buffer) map(tofrom: checksum)
    {
        // Create massive register pressure with many local variables
        register int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        register float f1, f2, f3, f4, f5, f6, f7, f8;
        register double d1, d2, d3, d4, d5;
        register char c1, c2, c3, c4;
        volatile int sink1, sink2, sink3;
        volatile double dsink1, dsink2;
        
        // Initialize from mapped arrays with complex addressing
        v1 = arr1[0][0];
        v2 = arr1[SIZE/2][SIZE/4];
        v3 = arr1[SIZE-1][SIZE-1];
        
        // Complex addressing modes
        int stride = SIZE;
        int offset = 32;
        
        for (int iter = 0; iter < ITERS; iter++) {
            // Force address computations into registers
            int idx1 = (iter * stride + offset) % SIZE;
            int idx2 = (iter * 3 + offset / 2) % SIZE;
            int idx3 = (idx1 * idx2 + iter) % SIZE;
            
            // Load many values creating live range interference
            v4 = arr1[idx1][idx2];
            v5 = arr1[idx2][idx3];
            v6 = arr1[idx3][idx1];
            
            // Chain computations to extend live ranges
            v7 = v1 + v2 * v3 - v4 / (v5 + 1);
            v8 = v2 * v3 + v4 - v5 * v6;
            v9 = v3 + v4 * v5 - v6 / (v7 + 1);
            v10 = v4 * v5 + v6 - v7 * v8;
            
            // Mixed type computations forcing mode conversions
            f1 = (float)v1 / 3.14f;
            f2 = (float)v2 * 2.718f;
            f3 = f1 + f2 * (float)v3;
            f4 = f2 - f1 / (float)(v4 + 1);
            
            d1 = arr2[idx1][idx2];
            d2 = arr2[idx2][idx3];
            d3 = d1 * 1.414 + d2 / 2.718;
            d4 = (double)v5 * d1 + (double)v6 * d2;
            d5 = d3 * d4 - d1 / (d2 + 0.001);
            
            // Struct member access with pointer chain
            int struct_idx = iter % 8;
            int data_idx = (iter * 7) % 16;
            int matrix_i = iter % 4;
            int matrix_j = (iter * 3) % 4;
            
            v1 = cs[struct_idx].data[data_idx];
            d1 = cs[struct_idx].matrix[matrix_i][matrix_j];
            
            // Pointer arithmetic forcing address reloads
            char *ptr = cs[struct_idx].ptr;
            c1 = ptr[iter % 64];
            c2 = ptr[(iter * 2) % 64];
            c3 = ptr[(iter * 3) % 64];
            c4 = c1 + c2 - c3;
            
            // Inline assembly with register constraints
            // Force specific register allocation conflicts
            asm volatile (
                "add %0, %1, %2\n\t"
                "mul %0, %0, %3"
                : "=r"(v1)
                : "r"(v2), "r"(v3), "r"(v4)
                : "cc"
            );
            
            // Mixed register class constraints
            int temp_int;
            double temp_double;
            asm volatile (
                "fcvt %s0, %w1\n\t"
                "fmul %d0, %d0, %d2"
                : "=w"(temp_double)
                : "r"(v5), "w"(d1)
                : 
            );
            
            // Memory constraints forcing address reloads
            asm volatile (
                "ldr %0, [%1, %2, lsl #2]\n\t"
                "str %0, [%3, %4, lsl #2]"
                : "=&r"(v6)
                : "r"(arr1), "r"(idx1), "r"(arr1), "r"(idx2)
                : "memory"
            );
            
            // Force output reloads with volatile stores
            sink1 = v1 + v2 + v3;
            sink2 = v4 * v5 - v6;
            sink3 = v7 + v8 + v9 + v10;
            
            dsink1 = d1 + d2 + d3;
            dsink2 = d4 * d5;
            
            // Complex array assignment forcing address computation
            arr1[(idx1 + idx2) % SIZE][(idx3 + iter) % SIZE] = v1 + v2;
            arr2[matrix_i][matrix_j] = d1 * d2 + d3;
            
            // Call helper function with many register arguments
            int func_result = helper_func(v1, v2, v3, v4, v5, v6, v7, v8);
            
            // More complex addressing with multiple computations
            int complex_idx = (v1 * stride + v2 * 3 + v3) % SIZE;
            double complex_val = arr2[complex_idx % SIZE][(complex_idx * 2) % SIZE];
            
            // Force secondary reload scenarios
            // Using builtins that may need specific registers
            int popcnt_result = __builtin_popcount(v1 | v2 | v3);
            
            // More inline asm with scarce register constraints
            float float_result;
            asm volatile (
                "fadd %s0, %s1, %s2\n\t"
                "fcvt %s0, %w3"
                : "=w"(float_result)
                : "w"(f1), "w"(f2), "r"(popcnt_result)
            );
            
            // Update checksum
            checksum += v1 + v2 + v3 + v4 + v5 + func_result;
            checksum += (int)(d1 + d2 + d3 + d4 + d5);
            checksum += c1 + c2 + c3 + c4;
            checksum += (int)float_result;
        }
        
        // Final complex computation using all variables
        int final_result = 0;
        for (int i = 0; i < 10; i++) {
            // Force all variables to be live simultaneously
            final_result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
            final_result += (int)(f1 + f2 + f3 + f4);
            final_result += (int)(d1 + d2 + d3 + d4 + d5);
            final_result += c1 + c2 + c3 + c4;
            
            // Rotate values to extend live ranges
            int temp = v1;
            v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
            v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = temp;
        }
        
        checksum += final_result;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

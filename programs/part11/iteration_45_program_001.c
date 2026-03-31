#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>

#define SIZE 128
#define NUM_VARS 25

struct DataNode {
    int data[SIZE];
    double values[SIZE];
    struct DataNode *next;
};

// Force no inlining to create function call pressure
__attribute__((noinline)) 
int helper_func(int a, int b, int c, int d, int e, float f, double g, char h) {
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
    return result + (int)f + (int)g + (int)h;
}

int main() {
    // Initialize complex data structures
    int matrix[SIZE][SIZE];
    double arr3d[SIZE][SIZE][SIZE];
    struct DataNode nodes[SIZE];
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
            for (int k = 0; k < SIZE; k++) {
                arr3d[i][j][k] = (i * 1.5 + j * 2.3 + k * 3.7);
            }
        }
        nodes[i].next = (i < SIZE - 1) ? &nodes[i + 1] : NULL;
        for (int j = 0; j < SIZE; j++) {
            nodes[i].data[j] = i * 1000 + j;
            nodes[i].values[j] = i * 0.001 + j * 0.0001;
        }
    }
    
    int result = 0;
    
    #pragma omp target map(to: matrix, arr3d, nodes) map(from: result)
    {
        // Create many local variables to consume registers
        register int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
        float f0, f1, f2, f3, f4, f5;
        double d0, d1, d2, d3, d4;
        char c0, c1, c2, c3;
        volatile int sink;  // For forcing output reloads
        
        // Initialize from mapped arrays with complex addressing
        v0 = matrix[0][0];
        v1 = matrix[1][SIZE-1];
        v2 = matrix[SIZE/2][SIZE/3];
        
        // Complex index calculations
        int stride = SIZE / 4;
        int offset = SIZE / 8;
        
        // Nested loops creating register pressure
        for (int i = 1; i < SIZE/16; i++) {
            for (int j = 1; j < SIZE/16; j++) {
                // Complex array indexing forcing address reloads
                int idx1 = (i * stride + j * 3 + offset) % SIZE;
                int idx2 = (j * stride + i * 5 - offset) % SIZE;
                int idx3 = (i * j * 7) % SIZE;
                
                // Load with complex addressing
                v3 = matrix[idx1][idx2];
                v4 = nodes[idx3].data[(i * j) % SIZE];
                
                // Pointer chain access
                struct DataNode *current = &nodes[idx1];
                if (current->next && current->next->next) {
                    v5 = current->next->next->data[idx2];
                }
                
                // Mixed type computations
                f0 = (float)v3 * 1.5f;
                f1 = (float)v4 * 2.3f;
                d0 = arr3d[i][j][(i+j)%SIZE];
                d1 = arr3d[j][i][(i*j)%SIZE];
                
                // Chain computations keeping many variables live
                v6 = v0 + v1 + v2;
                v7 = v3 * v4 - v5;
                v8 = (v6 << 3) | (v7 & 0xFF);
                v9 = v8 ^ (v0 * v1);
                
                // Force register constraints with inline assembly
                asm volatile (
                    "add %w0, %w1, %w2\n\t"
                    "mul %w0, %w0, %w3"
                    : "=r"(v0)
                    : "r"(v6), "r"(v7), "r"(v8)
                    : "cc"
                );
                
                // More inline asm with different constraints
                int temp;
                asm volatile (
                    "mov %w0, %w1\n\t"
                    "add %w0, %w0, #1"
                    : "=r"(temp)
                    : "r"(v9)
                );
                
                // Force output reload to memory with volatile
                sink = temp;
                
                // Complex assignment to array element (output reload)
                matrix[(i + j) % SIZE][(i - j + SIZE) % SIZE] = v0 + v1 + v2;
                
                // Mixed precision operations forcing mode conversions
                d2 = d0 + (double)f0;
                d3 = d1 * (double)v3;
                f2 = (float)d2 + f1;
                
                // More inline asm with floating point constraints
                // This may trigger secondary reloads on some architectures
                double dresult;
                asm volatile (
                    "fadd %d0, %d1, %d2"
                    : "=w"(dresult)
                    : "w"(d2), "w"(d3)
                );
                
                // Force integer to float move (potential secondary reload)
                float fresult;
                int ival = v4 + v5;
                asm volatile (
                    "scvtf %s0, %w1"
                    : "=w"(fresult)
                    : "r"(ival)
                );
                
                // Function call with many arguments (register pressure)
                v1 = helper_func(v0, v1, v2, v3, v4, f0, d0, (char)v5);
                
                // Update many variables to keep them live
                v2 += v3;
                v3 ^= v4;
                v4 |= v5;
                v5 &= v6;
                f3 = f0 + f1;
                f4 = f2 * 2.0f;
                d4 = d0 / d1;
                c0 = (char)(v0 & 0xFF);
                c1 = (char)(v1 & 0xFF);
                
                // Complex expression with mixed types
                result += v0 + (int)f0 + (int)d0 + c0;
                
                // Another volatile store
                volatile double dsink;
                dsink = dresult + fresult;
                
                // Pointer dereference assignment (output reload)
                int *ptr = &matrix[i][j];
                *ptr = v0 + v1 + (int)(f0 * 10.0f);
            }
        }
        
        // Final computation using all variables
        int final = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9
                  + (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4
                  + (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4
                  + c0 + c1 + c2 + c3;
        
        result = final % 1000000;
    }
    
    printf("Result: %d\n", result);
    return 0;
}

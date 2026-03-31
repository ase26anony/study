#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define NUM_VARS 25

struct DataNode {
    int index;
    float value;
    double data[SIZE];
    struct DataNode *next;
};

__attribute__((noinline))
double helper_func(int a, float b, double c, int d, float e, double f, 
                   int g, float h, double i, int j) {
    volatile double result = 0.0;
    result = (double)a + (double)b + c + (double)d + (double)e + f + 
             (double)g + (double)h + i + (double)j;
    
    // Force register usage with inline asm
    asm volatile ("# Helper function start" : : : "memory");
    asm volatile ("# Helper function end" : : : "memory");
    
    return result;
}

int main() {
    // Initialize complex data structures
    int matrix[SIZE][SIZE];
    double big_array[SIZE * 2][SIZE / 2];
    struct DataNode nodes[SIZE];
    volatile int checksum = 0;
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
        nodes[i].index = i;
        nodes[i].value = i * 0.5f;
        nodes[i].next = (i < SIZE - 1) ? &nodes[i + 1] : NULL;
        for (int k = 0; k < SIZE; k++) {
            nodes[i].data[k] = i * 100.0 + k * 0.1;
        }
    }
    
    for (int i = 0; i < SIZE * 2; i++) {
        for (int j = 0; j < SIZE / 2; j++) {
            big_array[i][j] = i * 0.3 + j * 0.7;
        }
    }
    
    #pragma omp target map(to: matrix, nodes, big_array) map(tofrom: checksum)
    {
        // Declare many local variables to create register pressure
        int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
        float fvar1, fvar2, fvar3, fvar4, fvar5, fvar6, fvar7, fvar8;
        double dvar1, dvar2, dvar3, dvar4, dvar5, dvar6, dvar7;
        char cvar1, cvar2, cvar3, cvar4;
        volatile int vsink1, vsink2, vsink3;
        volatile double dsink1, dsink2;
        
        // Initialize from mapped arrays with complex addressing
        var1 = matrix[0][0];
        var2 = matrix[SIZE-1][SIZE-1];
        fvar1 = nodes[0].value;
        dvar1 = nodes[SIZE/2].data[SIZE/4];
        
        // Complex nested loops with register pressure
        for (int i = 1; i < SIZE - 1; i++) {
            for (int j = 1; j < SIZE - 1; j++) {
                // Complex addressing modes
                int idx1 = (i * SIZE + j) % SIZE;
                int idx2 = (i * 3 + j * 7) % (SIZE / 2);
                int idx3 = (i << 2) + (j >> 1);
                
                // Chain computations with many live variables
                var3 = matrix[i-1][j] + matrix[i+1][j];
                var4 = matrix[i][j-1] * matrix[i][j+1];
                var5 = var3 ^ var4;
                
                // Mixed type computations forcing mode conversions
                fvar2 = (float)var5 * 0.123f + fvar1;
                fvar3 = nodes[idx1].value * 2.0f - fvar2;
                
                // Pointer chain access
                struct DataNode *current = &nodes[i];
                for (int k = 0; k < 3 && current != NULL; k++) {
                    dvar2 = current->data[idx2] * 1.5;
                    current = current->next;
                }
                
                // Multi-dimensional array with complex index
                dvar3 = big_array[idx3][idx2 % (SIZE/4)] + dvar2;
                
                // Character operations mixed with other types
                cvar1 = (char)((var3 + var4) & 0xFF);
                cvar2 = (char)(idx1 & 0xFF);
                var6 = cvar1 * cvar2 + var5;
                
                // Inline assembly with register constraints
                // Force specific register usage
                asm volatile (
                    "add %0, %1, %2\n\t"
                    "sub %3, %4, %5"
                    : "=r"(var7), "=r"(var8)
                    : "r"(var3), "r"(var4), "r"(var5), "r"(var6)
                    : "cc"
                );
                
                // More mixed operations
                fvar4 = (float)var7 / 256.0f + fvar3;
                fvar5 = fvar4 * 3.14159f - nodes[j].value;
                
                // Force output reloads with volatile assignments
                vsink1 = var7 + var8;
                dsink1 = dvar3 * fvar5;
                
                // Complex assignment to array element (forces address computation)
                if (idx1 < SIZE && idx2 < SIZE) {
                    matrix[idx1][idx2 % SIZE] = var7 + var8 + idx3;
                }
                
                // More variables to increase pressure
                var9 = var7 | var8;
                var10 = var9 & ~var6;
                fvar6 = fvar5 + (float)var10;
                fvar7 = fvar6 * 0.987f;
                
                // Another inline asm with mixed constraints
                int temp1, temp2;
                asm volatile (
                    "mov %0, %1\n\t"
                    "mov %2, %3"
                    : "=r"(temp1), "=m"(temp2)
                    : "r"(var9), "m"(var10)
                );
                
                // Force secondary reloads by moving between register classes
                double dtemp;
                asm volatile (
                    "# Force FP register usage %0"
                    : "=w"(dtemp)
                    : "r"(var10)
                );
                
                dvar4 = dtemp + dvar3;
                dvar5 = dvar4 * 2.0 - big_array[i % (SIZE*2)][j % (SIZE/2)];
                
                // Character array indexing
                cvar3 = (char)((i * j) & 0xFF);
                cvar4 = cvar3 + cvar1 - cvar2;
                var1 = var1 + cvar4;  // Update earlier variable
                
                // More volatile stores for output reloads
                vsink2 = var9 ^ var10;
                dsink2 = dvar5 + fvar7;
                
                // Call helper function with many register arguments
                dvar6 = helper_func(var1, fvar1, dvar1, var2, fvar2, dvar2,
                                   var3, fvar3, dvar3, var4);
                
                // Final computation chain
                fvar8 = (float)dvar6 * 0.5f + fvar7;
                dvar7 = (double)fvar8 + dvar5 + dvar4;
                
                // Update checksum
                checksum += var7 + (int)fvar8 + (int)dvar7;
            }
        }
        
        // Additional computations outside loops
        for (int i = 0; i < 10; i++) {
            // More inline asm with constraints
            int a, b, c;
            asm volatile (
                "mul %0, %1, %2"
                : "=r"(a)
                : "r"(i), "r"(checksum)
                : "cc"
            );
            
            // Complex struct access
            double* data_ptr = nodes[i % SIZE].data;
            for (int j = 0; j < 5; j++) {
                // Pointer arithmetic forcing address reloads
                double val = *(data_ptr + j + (i % 10));
                vsink3 = (int)val + a;
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

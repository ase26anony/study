#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 20

volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;
volatile int g_inner_skip = 1;

int main() {
    int array[SIZE][SIZE];
    volatile int accumulator = 0;
    volatile int loop_control = 0;
    
    // Initialize array with pattern
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * 100 + j;
        }
    }
    
    // Complex nested loop structure with partial overlaps
    // Outer loop - Level 1
    for (int i = 0; i < SIZE; i += 2) {
        // This basic block is in outer loop only
        accumulator += array[i][0];
        g_volatile_counter++;
        
        // Conditional that creates partial overlap
        if (i % 3 != g_outer_skip) {
            // Middle loop - Level 2 (not fully contained in outer)
            int j = 0;
            while (j < SIZE) {
                // Basic block in middle loop only
                loop_control = array[i][j] % 7;
                
                // Inner loop - Level 3 (not fully contained in middle)
                for (int k = 0; k < CHUNK; k++) {
                    // Complex body with multiple basic blocks
                    if (k % 2 == g_inner_skip) {
                        accumulator += array[i][j] * k;
                        // Additional basic block for even k
                        if (accumulator > 1000000) {
                            accumulator = accumulator % 1000;
                        }
                    } else {
                        accumulator -= array[j][i] / (k + 1);
                        // Early continue creates another basic block
                        if (k == 5) continue;
                    }
                    
                    // Another basic block
                    g_volatile_counter += (k & 1);
                }
                
                // Middle loop update with conditional
                j += (loop_control > 3) ? 2 : 1;
                
                // Break creates additional basic block
                if (j > SIZE / 2 && i > SIZE / 3) {
                    break;
                }
            }
            
            // Label and goto to create more complex CFG
            if (accumulator < 0) {
                accumulator = -accumulator;
            }
        } else {
            // Alternate path in outer loop (no middle loop)
            // Do-while loop for mixed loop types
            int m = 0;
            do {
                accumulator += array[i][m] * 2;
                m++;
                if (m >= CHUNK) break;
            } while (m < SIZE / 4);
        }
        
        // Outer loop continues with another conditional
        if (i % 5 == 0) {
            // Another inner loop but with different bounds
            for (int n = SIZE - 1; n > 0; n -= 2) {
                accumulator ^= array[i][n];
                // Nested if-else inside
                if (accumulator & 1) {
                    accumulator >>= 1;
                } else {
                    accumulator <<= 1;
                }
            }
        }
    }
    
    // Additional loop nest with different structure
    volatile int dim = SIZE / 2;
    for (int x = 0; x < dim; x++) {
        // Conditional that sometimes skips the next level
        if (x % 4 != 2) {
            int y = x;
            while (y < dim) {
                // Mixed loop type inside
                for (int z = 0; z < dim - y; z++) {
                    if (z % 3 == 0) {
                        accumulator += array[x][y] * array[y][z];
                        // Nested conditional
                        if (accumulator > 5000000) {
                            accumulator /= 2;
                            goto reduce;
                        }
                    }
                    reduce:
                    if (z == dim - y - 1) {
                        accumulator -= 100;
                    }
                }
                y += (x % 3) + 1;
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return accumulator != 0 ? 0 : 1;
}

#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 20

volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;
volatile int g_inner_condition = 1;

int main() {
    int array[SIZE][SIZE];
    volatile int accumulator = 0;
    volatile int loop_control = 0;
    
    // Initialize array
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * j + 1;
        }
    }
    
    // Complex nested loop structure with partial overlaps
    // Outer loop - level 1
    for (int i = 0; i < SIZE; i += 2) {
        // Multiple basic blocks in outer loop body
        if (g_outer_skip) {
            // Skip sometimes based on volatile
            accumulator += i;
            continue;  // Creates separate basic block
        }
        
        // Middle loop - level 2 (not strictly contained)
        // This loop starts inside outer but may exit to different blocks
        int j = 0;
        while (j < SIZE) {
            // Multiple basic blocks in middle loop
            if (j % 3 == 0) {
                // Branch that sometimes skips inner loop
                accumulator += array[i][j];
                j += g_volatile_counter ? 2 : 1;
                continue;
            }
            
            // Inner loop - level 3 (not strictly contained in middle)
            // Only executes in some iterations of middle loop
            if (g_inner_condition || (j % 5 == 0)) {
                // Fixed iteration loop but with complex body
                for (int k = 0; k < CHUNK; k++) {
                    // Multiple basic blocks in inner loop
                    if (k % 2 == 0) {
                        accumulator += array[i][j] * k;
                        if (accumulator > 1000) {
                            // Early exit creates separate block
                            accumulator -= 500;
                            break;  // Creates exit block
                        }
                    } else {
                        accumulator -= array[j][i] / (k + 1);
                    }
                    
                    // Label and goto to create additional blocks
                    if (k == CHUNK/2) {
                        goto midpoint;
                    }
                    continue;
                    
                midpoint:
                    accumulator += 100;
                }
                
                // Post-inner loop block (not part of inner's bitmap)
                loop_control++;
            } else {
                // Alternative path without inner loop
                accumulator -= array[j][i];
            }
            
            // Middle loop increment with condition
            j += (accumulator % 7) + 1;
            if (j > SIZE/2 && loop_control > 50) {
                // Early exit from middle loop
                break;
            }
        }
        
        // Outer loop tail with another conditional
        if (i % 10 == 0) {
            // Do-while loop inside outer (different type)
            int m = 0;
            do {
                accumulator += m * i;
                m++;
                if (m > 5) break;
            } while (m < 8);
        }
        
        // Toggle volatile conditions
        g_outer_skip = (i % 17 == 0);
        g_inner_condition = !g_inner_condition;
    }
    
    // Second independent loop structure with different overlap pattern
    volatile int x = 0, y = 0, z = 0;
    
    // Loop A
    for (x = 0; x < 50; x++) {
        // Loop B partially overlaps with A
        if (x % 3 != 0) {
            for (y = x; y < x + 30 && y < 50; y++) {
                // Loop C partially overlaps with B
                if (y % 4 != 0) {
                    z = 0;
                    while (z < 10) {
                        accumulator += array[x][y] * z;
                        z += (x + y) % 3 + 1;
                        if (z > 5 && accumulator % 2 == 0) {
                            // Creates exit block not in while's main bitmap
                            goto partial_exit;
                        }
                    }
                    continue;
                    
                partial_exit:
                    accumulator /= 2;
                }
                
                // Block that's in B but not in C
                accumulator -= y;
            }
        } else {
            // Block that's in A but not in B
            accumulator += x * 2;
        }
        
        // Another inner loop starting here (D)
        if (x % 7 == 0) {
            int w;
            for (w = 0; w < 15; w++) {
                // This loop shares some blocks with A but not all
                if (w % 2 == 0) {
                    accumulator += array[w][x];
                }
            }
            // Block in D but not in the for loop
            accumulator += w;
        }
    }
    
    // Final computation to prevent elimination
    printf("Result: %d\n", accumulator);
    
    // Use result to prevent dead code elimination
    if (accumulator > 0) {
        return 0;
    } else {
        return 1;
    }
}

#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 20

volatile int g_outer_cond = 1;
volatile int g_middle_cond = 1;
volatile int g_inner_cond = 1;
volatile int g_counter = 0;
volatile int g_prevent_opt = 0;

int main() {
    int array[SIZE][SIZE];
    volatile int accumulator = 0;
    volatile int control_var = 0;
    
    // Initialize array with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * j + (i ^ j);
        }
    }
    
    // Outer loop - Level 1
    // This loop has multiple basic blocks due to internal control flow
    for (int i = 0; i < SIZE; i += 2) {
        // First basic block in outer loop
        accumulator += array[i][0];
        control_var = i % 3;
        
        // Conditional that creates partial overlap
        if (g_outer_cond && (i % 4 != 0)) {
            // Middle loop - Level 2 (not strictly contained in outer)
            // This loop starts inside the outer but may exit to outer's continuation
            int j = i;
            while (j < SIZE && g_middle_cond) {
                // Multiple basic blocks in middle loop
                if (j % 2 == 0) {
                    accumulator += array[i][j] * 2;
                    j += 3;
                    continue;  // Creates another basic block
                } else {
                    accumulator -= array[i][j];
                    j += 2;
                    
                    // Inner loop - Level 3 (not strictly contained in middle)
                    // Fixed iteration count but with conditional execution
                    for (int k = 0; k < CHUNK && g_inner_cond; k++) {
                        // Complex body with multiple basic blocks
                        if (k % 3 == 0) {
                            array[i][j] += k * accumulator;
                            g_counter++;
                            
                            // Nested if creates more basic blocks
                            if (g_counter % 5 == 0) {
                                accumulator >>= 1;
                                goto inner_label;  // Creates another basic block
                            }
                        } else if (k % 3 == 1) {
                            accumulator ^= array[i][j];
                            inner_label:
                            g_prevent_opt = k;
                        } else {
                            // Third branch for more bitmap complexity
                            accumulator |= (1 << (k % 8));
                            if (accumulator > 1000) {
                                accumulator = 0;
                                break;  // Early exit creates another basic block
                            }
                        }
                        
                        // Post-increment computation
                        array[i][j] += (k & 1);
                    }
                }
                
                // Middle loop continuation with volatile check
                if (accumulator > 500) {
                    accumulator = accumulator % 100;
                    g_middle_cond = (j < SIZE - 10);
                }
            }
            
            // Back to outer loop but not the start
            if (accumulator < 0) {
                accumulator = -accumulator;
            }
        } else {
            // Alternative path in outer loop (creates blocks outside middle loop)
            // Do-while loop for mixed loop types
            int temp = i;
            do {
                accumulator += temp;
                temp--;
                if (temp <= 0) break;
                
                // Another small inner loop in the else branch
                for (int m = 0; m < 5; m++) {
                    accumulator ^= (1 << m);
                    if (m == 3) continue;
                    g_counter += m;
                }
            } while (temp > 0 && g_outer_cond);
        }
        
        // Outer loop tail with another conditional
        switch (control_var) {
            case 0:
                accumulator += i * 2;
                break;
            case 1:
                accumulator -= i;
                // Fall through
            case 2:
                accumulator *= (i % 7 + 1);
                break;
            default:
                accumulator = accumulator >> 1;
        }
        
        // Final check in outer loop
        if (i % 10 == 0) {
            g_outer_cond = !g_outer_cond;
        }
    }
    
    // Additional loop nest with different structure
    // Triple nested loops with partial overlaps
    for (int x = 0; x < SIZE/2; x++) {
        // First part of outer loop body
        accumulator += x;
        
        if (x % 3 == 0) {
            // Middle loop that partially overlaps
            for (int y = x; y < x + CHUNK && y < SIZE; y++) {
                accumulator ^= array[x][y];
                
                // Inner loop that's not fully contained
                int z = 0;
                while (z < y && g_inner_cond) {
                    array[x][y] += z;
                    z += (x % 2 + 1);
                    
                    if (z > CHUNK/2) {
                        // Jump to label creates another basic block
                        goto partial_exit;
                    }
                    
                    accumulator += z;
                }
                partial_exit:
                g_counter++;
            }
        } else {
            // Different blocks for the outer loop
            accumulator -= x;
        }
        
        // More outer loop blocks
        if (accumulator % 2 == 0) {
            accumulator >>= 1;
        }
    }
    
    // Prevent dead code elimination
    printf("Result: %d (counter: %d)\n", accumulator, g_counter);
    
    // Use results to prevent optimization
    if (accumulator > 1000000 || g_counter < 0) {
        return 1;  // Should never happen
    }
    
    return 0;
}

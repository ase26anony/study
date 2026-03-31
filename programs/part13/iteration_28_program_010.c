#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_SIZE 50

volatile int g_outer_cond = 1;
volatile int g_middle_cond = 0;
volatile int g_inner_cond = 1;
volatile int g_counter = 0;
volatile int g_modifier = 3;

int main() {
    int array[SIZE][SIZE];
    int result = 0;
    
    // Initialize array
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * SIZE + j;
        }
    }
    
    // Complex nested loop structure with partial overlaps
    // Outer loop - Level 1
    for (int i = 0; i < SIZE; i += 2) {
        // Multiple basic blocks in outer loop body
        if (g_outer_cond) {
            // This block is in outer loop but NOT in middle loop
            result += array[i][0];
            g_counter++;
            
            // Middle loop - Level 2 (not strictly contained in outer)
            // This loop starts in one branch but continues in another
            int j = 0;
            while (j < INNER_SIZE) {
                // Multiple basic blocks in middle loop
                if (g_middle_cond) {
                    // Skip inner loop sometimes
                    result -= array[i][j];
                    j += g_modifier;
                    continue;  // Creates additional basic block
                }
                
                // Inner loop - Level 3 (not strictly contained in middle)
                // Fixed iteration count but with conditional break
                for (int k = 0; k < 10; k++) {
                    // Complex inner loop body with multiple blocks
                    if (g_inner_cond) {
                        result += array[i][j] * k;
                        
                        // Early exit creates separate basic block
                        if (result > 1000000) {
                            goto inner_done;
                        }
                    } else {
                        result -= array[i][j] / (k + 1);
                    }
                    
                    // Another basic block
                    g_counter++;
                }
                inner_done:
                
                // Post-inner loop block (in middle but not in inner)
                j++;
                if (j % 7 == 0) {
                    g_middle_cond = !g_middle_cond;
                }
            }
            
            // Another block in outer loop (not in middle)
            result *= 2;
        } else {
            // Alternative path in outer loop (creates partial overlap)
            // Different middle loop structure
            do {
                result += array[i][INNER_SIZE - 1];
                g_counter--;
                
                // Minimal inner loop here (different bitmap)
                for (int k = 5; k > 0; k--) {
                    result >>= 1;
                }
            } while (result < 1000 && g_counter > 0);
        }
        
        // Final outer loop block
        if (i % 3 == 0) {
            g_outer_cond = !g_outer_cond;
        }
    }
    
    // Additional loop nest with different overlap pattern
    volatile int x = 0;
    volatile int y = 0;
    volatile int z = 0;
    
    // Triple nested loops with mixed types
    for (x = 0; x < 20; x++) {
        // Conditional that splits the loop body
        if (x % 2 == 0) {
            y = 0;
            while (y < 15) {
                // Inner loop that sometimes executes
                if (g_inner_cond) {
                    for (z = 0; z < 10; z++) {
                        result += x * y * z;
                        // Break creates another basic block
                        if (z == x) break;
                    }
                }
                y += 1 + (x % 3);
            }
        } else {
            // Different inner structure
            for (y = 10; y > 0; y--) {
                result -= x * y;
            }
        }
        
        // Loop with goto creating complex CFG
        int temp = 0;
    retry:
        temp++;
        if (temp < 3) {
            result += temp;
            goto retry;
        }
    }
    
    printf("Result: %d\n", result);
    printf("Counter: %d\n", g_counter);
    
    return result != 0 ? 0 : 1;
}

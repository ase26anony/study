#include <stdio.h>
#include <stdlib.h>

#define SIZE 100

volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;
volatile int g_inner_condition = 1;

int main() {
    int array[SIZE][SIZE];
    volatile int accumulator = 0;
    
    // Initialize array with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * j + 1;
        }
    }
    
    // Outer loop - Level 1
    for (int i = 0; i < SIZE; i += 2) {
        // This block is in outer loop but not in middle loop
        accumulator += array[i][0];
        g_volatile_counter++;
        
        // Conditional that creates partial overlap
        if (g_outer_skip || (i % 3 != 0)) {
            // Middle loop - Level 2 (not strictly contained in outer)
            int j = i;
            while (j < SIZE && j < i + 20) {
                // Block in middle but not in outer
                if (j % 2 == 0) {
                    accumulator -= array[i][j];
                } else {
                    accumulator += array[i][j] * 2;
                }
                
                // Inner loop - Level 3 (not strictly contained in middle)
                for (int k = 0; k < 10; k++) {
                    // Complex inner loop body with multiple basic blocks
                    if (g_inner_condition && (k % 3 == 0)) {
                        array[i][j] += k;
                        accumulator += array[i][j];
                        
                        // Early exit sometimes
                        if (accumulator > 10000) {
                            goto inner_loop_label;
                        }
                    } else {
                        array[i][j] -= k;
                        accumulator -= array[i][j];
                    }
                    
                    // Another basic block in inner loop
                    g_volatile_counter += (k & 1);
                }
                inner_loop_label:
                
                // Continue or break based on volatile
                if (g_volatile_counter > 1000) {
                    break;
                }
                
                j += (i % 4) + 1;  // Variable increment
            }
        } else {
            // Alternative path in outer loop (not in middle loop)
            for (int alt = 0; alt < 5; alt++) {
                accumulator += alt * i;
            }
        }
        
        // Another block in outer but not in middle
        if (accumulator < 0) {
            accumulator = 0;
        }
    }
    
    // Additional nested structure with do-while
    int x = 0;
    do {
        volatile int y = 0;
        while (y < 5) {
            for (int z = 0; z < 3; z++) {
                // Mix of loop types in nesting
                accumulator += x * y * z;
                if (accumulator % 7 == 0) {
                    continue;  // Creates additional basic blocks
                }
                g_volatile_counter++;
            }
            y += (x % 2) + 1;
        }
        x++;
    } while (x < 10 && g_volatile_counter < 2000);
    
    printf("Result: %d (volatile counter: %d)\n", accumulator, g_volatile_counter);
    return accumulator != 0 ? 0 : 1;
}

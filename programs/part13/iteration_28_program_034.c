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
    
    // Initialize array with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * j + (i ^ j);
        }
    }
    
    // Complex nested loop structure with partial overlaps
    // Outer loop - level 1
    for (int i = 0; i < SIZE; i += 2) {
        // Multiple basic blocks in outer loop body
        if (g_outer_skip) {
            // This block is in outer loop but NOT in middle loop
            accumulator += i * 3;
            continue;  // Creates separate basic block
        }
        
        // Middle loop - level 2 (not strictly contained in outer)
        // This loop starts in outer but has blocks outside
        int j = 0;
        while (j < SIZE) {
            // Multiple basic blocks in middle loop body
            if (j % 3 == 0) {
                // Block in middle but NOT in inner
                accumulator += array[i][j];
                j += 2;
                continue;
            }
            
            // Inner loop - level 3 (not strictly contained in middle)
            // Only executes in some iterations
            if (g_inner_condition || (i + j) % 5 == 0) {
                // Fixed iteration count to encourage hardware loops
                for (int k = 0; k < CHUNK; k++) {
                    // Complex inner loop body with multiple blocks
                    if (k % 2 == 0) {
                        accumulator += array[i][j] * k;
                        // goto label creates additional basic block
                        if (accumulator > 1000) {
                            accumulator = accumulator % 1000;
                        }
                    } else {
                        accumulator -= array[j][i] / (k + 1);
                    }
                    
                    // Break statement creates exit block
                    if (accumulator < -500) {
                        accumulator = 0;
                        break;
                    }
                }
            } else {
                // Alternative path in middle loop (not in inner)
                accumulator += j * 7;
            }
            
            j += (i % 4) + 1;  // Variable increment
        }
        
        // Another block in outer loop but not in middle
        if (i % 7 == 0) {
            g_volatile_counter++;
            // do-while loop with volatile condition
            do {
                accumulator += g_volatile_counter;
                loop_control = rand() % 10;
            } while (loop_control > 5);
        }
    }
    
    // Additional loop nest with different structure
    // Creates more complex bitmap relationships
    volatile int x = 0, y = 0, z = 0;
    
    for (x = 0; x < 50; x++) {
        // Conditional that sometimes skips the next loop
        if (x % 3 != 0) {
            // Loop that partially overlaps with outer
            y = x;
            while (y < 50) {
                // Inner loop that's conditionally executed
                if (accumulator % 2 == 0) {
                    for (z = 0; z < 10; z++) {
                        // Multiple operations to create blocks
                        accumulator += (x * y * z);
                        if (z == 5) {
                            accumulator -= 100;
                            // continue creates new block
                            continue;
                        }
                        accumulator += 1;
                    }
                } else {
                    // Path in middle loop not taken by inner
                    accumulator += y * 11;
                }
                
                // Break with condition
                if (y > 40 && accumulator > 2000) {
                    break;
                }
                y += (x % 3) + 1;
            }
        }
        
        // Another inner loop in different branch
        if (x % 4 == 0) {
            for (int w = 0; w < 8; w++) {
                accumulator += w * x;
                // Nested if-else
                if (w % 2 == 0) {
                    if (x % 2 == 0) {
                        accumulator += 2;
                    } else {
                        accumulator -= 1;
                    }
                }
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return accumulator != 0 ? 0 : 1;
}

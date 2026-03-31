#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 20

volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;

int main() {
    int array[SIZE][SIZE];
    volatile int accumulator = 0;
    volatile int loop_control = 1;
    
    // Initialize array with pattern
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * SIZE + j;
        }
    }
    
    // Outer loop - Level 1
    for (int i = 0; i < SIZE; i += 2) {
        // This if creates a basic block that's in outer loop but not in middle loop
        if (g_outer_skip) {
            accumulator += array[i][0];
            continue;  // Skip middle loop entirely sometimes
        }
        
        // Middle loop - Level 2 (not strictly contained in outer)
        // This loop starts in outer but has blocks outside outer's main path
        int j = 0;
        while (j < SIZE) {
            // Block that's in middle but not in outer's main path
            if (j % 3 == 0) {
                accumulator -= array[i][j];
                j += 2;
                continue;  // Creates separate basic block
            }
            
            // Inner loop - Level 3 (not strictly contained in middle)
            // This loop is inside an if within the middle loop
            if (loop_control && (j % 5 != 0)) {
                // Complex inner loop body with multiple blocks
                for (int k = 0; k < CHUNK; k++) {
                    // Multiple basic blocks within inner loop
                    if (k % 2 == 0) {
                        accumulator += array[i][j] * k;
                        if (accumulator > 1000) {
                            accumulator = accumulator % 1000;
                            // goto creates additional control flow
                            goto inner_label;
                        }
                    } else {
                        accumulator -= array[i][j] / (k + 1);
                    }
                    
                    inner_label:
                    // Another basic block
                    if (k == CHUNK - 1) {
                        g_volatile_counter++;
                    }
                    
                    // Early break creates another block
                    if (g_volatile_counter > 50) {
                        break;
                    }
                }
            } else {
                // Alternative path in middle loop (not containing inner loop)
                accumulator += array[i][j] >> 1;
            }
            
            j += (i % 4) + 1;  // Variable increment
        }
        
        // This block is in outer loop but after middle loop
        if (i % 10 == 0) {
            // Another loop inside outer but not nested with middle
            do {
                accumulator += i;
                g_volatile_counter--;
            } while (g_volatile_counter > 0 && loop_control);
        }
    }
    
    // Additional complex nesting
    volatile int toggle = 1;
    for (int a = 0; a < 50; a++) {
        if (toggle) {
            int b = 0;
            while (b < 30) {
                // Loop with switch inside
                for (int c = 0; c < 10; c++) {
                    switch (c % 3) {
                        case 0:
                            accumulator += a * b;
                            break;
                        case 1:
                            accumulator -= c;
                            // Nested if creates more blocks
                            if (b % 2) {
                                accumulator >>= 1;
                            }
                            break;
                        default:
                            accumulator ^= 0xFF;
                            break;
                    }
                    
                    // Another control structure
                    if (accumulator < 0) {
                        accumulator = -accumulator;
                        continue;
                    }
                }
                
                // This increment is in a separate block
                b += (a % 3) + 1;
                
                // Early exit from while
                if (accumulator > 10000) {
                    goto finish_while;
                }
            }
            finish_while:
            toggle = !toggle;
        }
    }
    
    printf("Result: %d\n", accumulator);
    return accumulator != 0 ? 0 : 1;
}

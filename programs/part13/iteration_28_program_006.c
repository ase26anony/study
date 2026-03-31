#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_SIZE 50

volatile int g_volatile_counter = 0;
volatile int g_condition = 1;

int main() {
    int array[SIZE][SIZE];
    volatile int accumulator = 0;
    volatile int outer_mod = 0;
    
    // Initialize array with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = (i * j) % 100;
        }
    }
    
    // Outer loop - Level 1
    for (int i = 0; i < SIZE; i += 2) {
        // Multiple basic blocks in outer loop body
        if (g_condition) {
            // This block is in outer loop but not in middle loop
            accumulator += i;
            outer_mod = i % 3;
            
            // Middle loop - Level 2 (not strictly contained in outer)
            // Only executes in one branch of the outer loop's conditional
            int j = 0;
            while (j < INNER_SIZE) {
                // Multiple basic blocks in middle loop body
                if (j % 2 == 0) {
                    // Inner loop - Level 3 (not strictly contained in middle)
                    for (int k = 0; k < 10; k++) {
                        // Complex inner loop body with multiple blocks
                        if (k % 3 == outer_mod) {
                            array[i][j] += k;
                            accumulator += array[i][j];
                            
                            // Early exit condition
                            if (accumulator > 10000) {
                                goto skip_inner;
                            }
                        } else {
                            array[i][j] -= k;
                            accumulator -= array[i][j];
                        }
                        
                        // Another basic block
                        g_volatile_counter++;
                    }
                    skip_inner:
                    
                    // Continue with middle loop
                    if (accumulator < 0) {
                        accumulator = 0;
                        break;  // Early exit from middle loop
                    }
                } else {
                    // Alternative path in middle loop
                    array[i][j] *= 2;
                    accumulator += array[i][j];
                    
                    // Nested if-else creating more basic blocks
                    if (g_volatile_counter % 5 == 0) {
                        continue;  // Skip to next iteration
                    }
                }
                
                j += (g_volatile_counter % 3) + 1;  // Variable increment
                
                // Another basic block at end of middle loop
                if (j > INNER_SIZE / 2) {
                    g_condition = !g_condition;  // Flip condition for outer loop
                }
            }
        } else {
            // Alternative branch of outer loop - no middle loop here
            // This creates blocks in outer loop that are NOT in middle loop
            for (int x = 0; x < 5; x++) {
                accumulator -= x;
                if (accumulator < -1000) {
                    accumulator = 0;
                    goto reset_outer;
                }
            }
            reset_outer:
            continue;  // Skip to next outer iteration
        }
        
        // Final block of outer loop (after middle loop)
        accumulator %= 1000;
        if (accumulator == 777) {
            break;  // Rare early exit from outer loop
        }
    }
    
    // Additional loop structure with do-while
    int counter = 0;
    do {
        // This loop has partial overlap with previous structure
        volatile int local_volatile = counter;
        
        // Another nested loop with different bounds
        for (int m = 0; m < counter % 10; m++) {
            if (m % 2 == 0) {
                accumulator += m * local_volatile;
                if (accumulator > 5000) {
                    goto finish_all;
                }
            }
            
            // Label for goto creating additional basic block
            retry_point:
            local_volatile = (local_volatile * 13) % 97;
        }
        
        counter++;
        
        // Complex condition with volatile
        if (g_volatile_counter++ % 7 == 0) {
            goto retry_point;  // Unstructured control flow
        }
        
    } while (counter < 20 && accumulator < 10000);
    
    finish_all:
    
    // Print result to prevent optimization
    printf("Result: %d (volatile counter: %d)\n", accumulator, g_volatile_counter);
    
    return accumulator != 0 ? 0 : 1;
}

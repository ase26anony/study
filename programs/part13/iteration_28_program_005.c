#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_BOUND 50
#define MIDDLE_BOUND 75

volatile int g_volatile_counter = 0;
volatile int g_condition_seed = 7;

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
    for (int i = 0; i < SIZE; i++) {
        // Multiple basic blocks in outer loop body
        if (i % 3 == 0) {
            // This block is in outer loop but NOT in middle loop
            accumulator += array[i][0];
            outer_mod = 1;
        } else {
            outer_mod = 0;
        }
        
        // Complex condition with volatile to prevent optimization
        volatile int skip_middle = (g_condition_seed * i) % 5;
        
        // Middle loop - Level 2 (not strictly contained in outer)
        // Only enters middle loop 80% of the time
        if (skip_middle != 0) {
            int j = 0;
            // while loop for middle level (mixed loop type)
            while (j < MIDDLE_BOUND) {
                // Multiple basic blocks in middle loop
                if (j % 4 == 0) {
                    // Block in middle but NOT in inner loop
                    accumulator -= array[i][j];
                    j += 2;  // Skip increment
                    continue;  // Creates additional basic block
                }
                
                volatile int enable_inner = (g_condition_seed + j) % 3;
                
                // Inner loop - Level 3 (not strictly contained in middle)
                // Only enters inner loop 66% of the time
                if (enable_inner > 0) {
                    // for loop for inner level (mixed loop type)
                    for (int k = 0; k < INNER_BOUND; k++) {
                        // Complex inner loop body with multiple blocks
                        if (k % 5 == 0) {
                            // Block in inner but could be executed independently
                            accumulator += array[i][j] * k;
                            if (accumulator > 10000) {
                                // break creates additional control flow
                                accumulator = accumulator % 1000;
                            }
                        } else {
                            accumulator -= k;
                        }
                        
                        // Volatile modification prevents loop simplification
                        g_volatile_counter++;
                        
                        // Early exit condition creates another basic block
                        if (g_volatile_counter > 100000) {
                            goto early_exit;
                        }
                    }
                } else {
                    // Alternative path in middle loop (not in inner)
                    accumulator += j * 2;
                }
                
                j++;
            }
            
            // Another block in middle loop but outside inner
            if (outer_mod) {
                accumulator *= 2;
            }
        }
        
        // Another block in outer loop but outside middle
        if (i % 10 == 0) {
            // do-while loop (another loop type)
            int m = 0;
            do {
                accumulator += m;
                m++;
            } while (m < 5 && accumulator < 5000);
        }
        
        // Label for goto (creates additional basic block)
        early_exit_continue:
        if (i % 20 == 0) {
            accumulator /= 2;
        }
    }
    
    early_exit:
    // Final computation to prevent dead code elimination
    int result = accumulator % 1000;
    printf("Result: %d\n", result);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return result;
}

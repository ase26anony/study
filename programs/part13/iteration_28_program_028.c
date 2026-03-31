#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define HALF_SIZE (SIZE / 2)

volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;
volatile int g_inner_skip = 1;

int main() {
    int array1[SIZE][SIZE];
    int array2[SIZE][SIZE];
    volatile int accumulator = 0;
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array1[i][j] = i * j;
            array2[i][j] = i + j;
        }
    }
    
    // Complex nested loop structure with partial overlaps
    // Outer loop - Level 1
    for (int outer = 0; outer < SIZE; outer++) {
        // Create multiple basic blocks in outer loop body
        if (outer % 3 == 0) {
            // Skip inner loops sometimes
            g_outer_skip = 1;
            continue;
        }
        
        g_outer_skip = 0;
        
        // Conditional block that partially contains middle loop
        if (outer < HALF_SIZE) {
            // Middle loop - Level 2 (not fully contained in outer)
            // This loop starts in outer but may exit to outer's continuation
            int middle = 0;
            while (middle < HALF_SIZE) {
                // Multiple basic blocks in middle loop
                if (middle % 4 == 0) {
                    // Early continue creates separate basic block
                    middle += g_volatile_counter;
                    continue;
                }
                
                // Another conditional creating more blocks
                if (middle > outer) {
                    // Inner loop - Level 3 (not fully contained in middle)
                    // This creates partial overlap scenario
                    for (int inner = 0; inner < middle; inner++) {
                        // Complex inner loop body with multiple blocks
                        if (inner % 2 == 0) {
                            // Skip some iterations
                            accumulator += array1[outer][inner];
                            goto inner_label;
                        } else {
                            accumulator -= array2[middle][inner];
                        }
                        
                        // Label and goto create additional basic blocks
                        inner_label:
                        if (g_inner_skip && inner > HALF_SIZE/2) {
                            // Early break from inner loop
                            break;
                        }
                        
                        // Dummy computation
                        accumulator += (outer * middle + inner) % 7;
                    }
                } else {
                    // Alternative path in middle loop (no inner loop)
                    accumulator += array1[outer][middle] * 2;
                }
                
                // Middle loop increment with volatile check
                middle += 1 + (g_volatile_counter % 2);
                
                // Potential early exit
                if (accumulator > 10000) {
                    goto middle_exit;
                }
            }
            middle_exit:
            // Control returns to outer loop
            ;
        } else {
            // Outer loop's else branch (no middle loop here)
            // This creates blocks in outer that are NOT in middle
            accumulator += outer * outer;
            
            // Do-while loop with different structure
            int k = 0;
            do {
                if (k % 5 == 0) {
                    accumulator -= k;
                }
                k++;
            } while (k < 10 && g_volatile_counter < 100);
        }
        
        // Outer loop continuation after conditional
        // This block is in outer but not in middle when middle wasn't entered
        if (accumulator < 0) {
            accumulator = -accumulator;
        }
        
        // Volatile modification affects future iterations
        g_volatile_counter = (g_volatile_counter + 1) % 10;
    }
    
    // Additional loop nest with different overlap pattern
    // Creates another scenario for bitmap analysis
    for (int i = 0; i < SIZE; i += 2) {
        // Loop with switch inside
        for (int j = i; j < SIZE; j++) {
            switch (j % 3) {
                case 0:
                    // Nested short loop
                    for (int k = 0; k < 5; k++) {
                        accumulator += (i + j + k) % 11;
                        if (k == 3) break;
                    }
                    break;
                case 1:
                    accumulator += j;
                    break;
                default:
                    // Empty case - still a basic block
                    break;
            }
            
            // Conditional continue
            if (j % 7 == 0) continue;
            
            // Small while loop
            int w = 0;
            while (w < 3 && g_inner_skip) {
                accumulator += w;
                w++;
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return accumulator != 0 ? 0 : 1;
}

#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 20

volatile int g_volatile_counter = 0;
volatile int g_outer_control = 1;
volatile int g_inner_skip = 0;

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
    
    // Complex nested loop structure with partial overlaps
    // Outer loop - Level 1
    for (int outer = 0; outer < SIZE; outer += CHUNK) {
        g_volatile_counter++;
        
        // First conditional branch - sometimes skips middle loop
        if (g_outer_control || (outer % 3 != 0)) {
            // Middle loop - Level 2 (not fully contained in outer)
            // This loop starts in outer but has blocks outside
            int middle = 0;
            volatile int mid_control = outer % 2;
            
            // Mixed loop type: do-while with variable condition
            do {
                // Multiple basic blocks in middle loop body
                if (middle % 5 == 0) {
                    accumulator += array[outer][middle];
                    // Early continue creates additional basic block
                    if (mid_control && (middle > CHUNK)) {
                        middle += 2;
                        continue;
                    }
                } else {
                    accumulator -= array[outer][middle];
                }
                
                // Inner loop - Level 3 (not fully contained in middle)
                // Only executes in some iterations
                if (!g_inner_skip || (middle % 7 != 0)) {
                    // Inner for loop with constant bounds
                    for (int inner = 0; inner < CHUNK; inner++) {
                        // Complex inner loop body with multiple blocks
                        volatile int inner_flag = inner % 3;
                        
                        if (inner_flag == 0) {
                            array[outer + inner/2][middle] += accumulator;
                            // Nested if creates more blocks
                            if (inner > CHUNK/2) {
                                accumulator >>= 1;
                            }
                        } else if (inner_flag == 1) {
                            // Label and goto create additional control flow
                            if (inner % 11 == 0) {
                                array[outer][middle + inner/3] *= 2;
                                // Skip to update
                                goto inner_update;
                            }
                            accumulator ^= array[outer][inner];
                        } else {
                            // Default case
                            accumulator |= 0x1;
                        }
                        
                        inner_update:
                        // Empty statement for label target
                        ;
                        
                        // Break can create exit block
                        if (accumulator > 1000000) {
                            accumulator = 0;
                            break;
                        }
                    } // End inner loop
                }
                
                middle++;
                // Loop condition depends on volatile
            } while (middle < (SIZE - outer % CHUNK) && loop_control);
            
            // Code block after middle loop but still in outer
            accumulator += outer * middle;
        } else {
            // Alternative path in outer loop (no middle loop)
            // This creates blocks in outer that aren't in middle
            for (int k = 0; k < CHUNK/2; k++) {
                accumulator -= k;
            }
        }
        
        // Second middle loop in same outer - creates different overlap
        // This one is a while loop
        volatile int second_mid = outer;
        while (second_mid < outer + CHUNK && second_mid < SIZE) {
            // Another inner loop with different bounds
            for (int inner2 = second_mid; inner2 > 0; inner2--) {
                if (inner2 % 4 == 0) {
                    array[outer][second_mid] += inner2;
                    continue;
                }
                accumulator += array[inner2][second_mid];
                
                // Early exit from inner loop
                if (accumulator < -1000000) {
                    accumulator = 0;
                    break;
                }
            }
            second_mid += (outer % 3) + 1;
        }
    }
    
    // Additional loop nest with different pattern
    // Three-level nest where each partially overlaps
    volatile int control_a = 1, control_b = 1, control_c = 1;
    
    for (int a = 0; a < SIZE/2 && control_a; a += 4) {
        // Conditional that sometimes contains next loop
        if (a % 8 == 0) {
            int b = a;
            while (b < SIZE && control_b) {
                // Loop with multiple exit points
                for (int c = 0; c < CHUNK; c++) {
                    if (c % 6 == 0) {
                        array[a][b] += c;
                        if (array[a][b] > 1000) {
                            // Exit middle loop from inner
                            control_b = 0;
                            break;
                        }
                    } else {
                        accumulator += array[b][c];
                    }
                    
                    // Volatile check prevents optimization
                    if (g_volatile_counter > 50) {
                        control_c = 0;
                    }
                }
                
                // Update with volatile dependency
                b += (control_c ? 2 : 3);
            }
        } else {
            // Different path in outer loop
            for (int d = 0; d < a; d++) {
                accumulator ^= array[d][a];
            }
        }
        
        // Update volatile to affect inner loops
        g_volatile_counter += a;
    }
    
    // Final computation to prevent elimination
    int result = 0;
    for (int i = 0; i < SIZE; i += 10) {
        for (int j = 0; j < SIZE; j += 10) {
            result += array[i][j];
        }
    }
    
    printf("Result: %d (Accumulator: %d)\n", result, accumulator);
    return result % 256;
}

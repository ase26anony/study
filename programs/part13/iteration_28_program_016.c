#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 20

volatile int g_volatile_counter = 0;
volatile int g_control = 1;

int main() {
    // Multi-dimensional arrays to work with
    int matrix[SIZE][SIZE];
    int result[SIZE] = {0};
    
    // Initialize with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * j) % 100;
        }
    }
    
    // Outer loop with multiple basic blocks
    for (int outer = 0; outer < SIZE; outer += CHUNK) {
        // First basic block in outer loop
        g_volatile_counter++;
        
        // Conditional that creates partial overlap
        if (g_control & 0x1) {
            // Middle loop - NOT strictly contained in outer loop
            // because it starts in a conditional block
            int middle = 0;
            volatile int mid_control = outer % 3;
            
            // do-while for mixed loop type
            do {
                // Multiple basic blocks in middle loop
                if (middle % 2 == 0) {
                    // Inner loop - NOT strictly contained in middle loop
                    // because of the break statement placement
                    for (int inner = 0; inner < CHUNK; inner++) {
                        // Complex inner loop body with multiple blocks
                        volatile int inner_check = matrix[outer + inner][middle];
                        
                        if (inner_check > 50) {
                            result[outer] += inner_check;
                            // continue creates another basic block
                            continue;
                        } else {
                            result[outer] -= inner_check;
                            // break could exit early
                            if (inner_check < 10 && g_volatile_counter > 100) {
                                break;
                            }
                        }
                        
                        // Another basic block
                        g_volatile_counter += inner;
                    }
                } else {
                    // Alternative path in middle loop
                    result[outer] *= 2;
                    // goto creates interesting control flow
                    if (middle == CHUNK/2) {
                        goto skip_point;
                    }
                }
                
            skip_point:
                middle++;
                
                // while condition depends on volatile
            } while (middle < CHUNK && g_control > 0);
            
            // Block after middle loop but still in outer loop's if
            g_volatile_counter += 100;
        } else {
            // Alternative branch in outer loop
            // Another loop here creates different bitmap relationships
            int alt = 0;
            while (alt < CHUNK/2) {
                result[outer] += alt;
                alt++;
                if (g_volatile_counter > 200) {
                    // break creates another basic block
                    break;
                }
            }
        }
        
        // Final block in outer loop
        // Nested switch for more basic blocks
        switch (outer % 4) {
            case 0:
                g_control ^= 1;
                break;
            case 1:
                g_control |= 2;
                break;
            case 2:
                g_control &= ~1;
                break;
            default:
                g_control = (g_control << 1) | 1;
        }
    }
    
    // Additional loop nest with different structure
    // This creates more opportunities for bitmap intersection analysis
    volatile int acc = 0;
    for (int i = 0; i < SIZE/2; i++) {
        // Conditional with loop in one branch only
        if (i % 3 == 0) {
            int j = i;
            // while loop with complex exit condition
            while (j < SIZE && acc < 1000) {
                // Inner for loop
                for (int k = 0; k < 5; k++) {
                    acc += matrix[i][j] * k;
                    // Early exit based on volatile
                    if (g_volatile_counter > 500) {
                        goto early_exit;
                    }
                }
                j += 2;
            }
        early_exit:
            // Label creates another basic block
            result[i] += acc;
        }
    }
    
    // Compute final result to prevent optimization
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += result[i];
        final_sum ^= g_volatile_counter;
    }
    
    printf("Result: %d (volatile counter: %d)\n", final_sum, g_volatile_counter);
    
    // Return value prevents dead code elimination
    return (final_sum > 0) ? 0 : 1;
}

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
            array[i][j] = i * 100 + j;
        }
    }
    
    // Complex nested loop structure with partial overlaps
    // Outer loop - Level 1
    for (int outer = 0; outer < SIZE; outer += CHUNK) {
        g_volatile_counter++;
        
        // Multiple basic blocks in outer loop body
        if (outer % 40 == 0) {
            // Skip inner loops for some iterations
            g_outer_skip = 1;
            accumulator += outer * 1000;
            continue;  // Creates additional basic block
        } else {
            g_outer_skip = 0;
            
            // Middle loop - Level 2 (not fully contained in outer)
            // This loop starts in outer but may extend beyond
            int middle_start = outer;
            int middle_end = (outer + CHUNK * 2) % SIZE;
            
            // do-while for variety
            int middle = middle_start;
            do {
                // Multiple basic blocks in middle loop
                if (middle % 3 == 0) {
                    // Early continue creates another basic block
                    accumulator += array[outer][middle];
                    middle++;
                    continue;
                }
                
                // Inner loop - Level 3 (not fully contained in middle)
                // This creates partial overlap scenario
                if (g_inner_condition || (middle % 7 == 0)) {
                    // for loop with constant bounds
                    for (int inner = 0; inner < CHUNK; inner++) {
                        // Complex inner loop body with multiple blocks
                        volatile int temp = array[outer][inner] + array[middle][inner];
                        
                        if (temp % 2 == 0) {
                            accumulator += temp;
                            // goto creates additional control flow
                            if (accumulator > 1000000) {
                                goto adjust_accumulator;
                            }
                        } else {
                            accumulator -= temp / 2;
                        }
                        
                        // Another conditional block
                        if (inner == CHUNK / 2) {
                            loop_control = middle * inner;
                        }
                        
                        adjust_accumulator:
                        // Label for goto target
                        if (accumulator < 0) {
                            accumulator = accumulator * -1;
                        }
                    }
                } else {
                    // Alternative path without inner loop
                    accumulator += middle * 100;
                }
                
                // Break condition based on volatile
                if (g_volatile_counter > 50) {
                    break;
                }
                
                middle++;
                if (middle >= SIZE) middle = 0;
                
            } while (middle != middle_end);
        }
        
        // Another conditional at outer loop level
        switch (outer % 4) {
            case 0:
                accumulator += 1;
                break;
            case 1:
                accumulator += 2;
                // fall through
            case 2:
                accumulator += 3;
                break;
            default:
                accumulator += 4;
        }
    }
    
    // Additional loop nest with while loops
    int x = 0, y = 0, z = 0;
    volatile int limit = SIZE / 2;
    
    while (x < limit) {
        // Mixed loop types
        for (y = x; y < limit + x; y++) {
            if (y >= SIZE) break;
            
            do {
                z = (z + 1) % CHUNK;
                accumulator += array[x][y] * z;
                
                // Nested conditional with goto
                if (accumulator % 1000 == 0) {
                    goto skip_increment;
                }
                
                z++;
                
                skip_increment:
                // Empty label for goto target
                ;
                
            } while (z < CHUNK / 2 && g_inner_condition);
            
            // Partial overlap: this block is in middle loop
            // but not in the do-while above
            if (y % 5 == 0) {
                accumulator -= array[y][x];
            }
        }
        
        // This block is in outer while but not in the for loop
        x += (g_volatile_counter % 3) + 1;
        if (x > SIZE / 4) {
            g_inner_condition = 0;
        }
    }
    
    // Final computation to prevent elimination
    printf("Result: %d\n", accumulator % 10000);
    return accumulator % 10000;
}

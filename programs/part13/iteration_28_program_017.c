#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 20

volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;
volatile int g_inner_skip = 0;

int main() {
    int data[SIZE][SIZE];
    volatile int accumulator = 0;
    volatile int loop_control = 1;
    
    // Initialize array
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            data[i][j] = i * j + (i ^ j);
        }
    }
    
    // Complex nested loop structure with partial overlaps
    // Outer loop - Level 1
    for (int outer = 0; outer < SIZE; outer += CHUNK) {
        g_volatile_counter++;
        
        // First basic block in outer loop
        if (g_outer_skip > 0) {
            accumulator += data[outer][0];
            continue;  // Creates separate basic block
        }
        
        // Middle loop - Level 2 (not fully contained in outer)
        int middle = 0;
        volatile int mid_cond = outer % 3;
        
        // This while loop creates blocks that may not be in outer's bitmap
        while (middle < SIZE) {
            // Basic block inside middle but potentially outside outer
            if (middle % 2 == 0) {
                accumulator += data[outer][middle];
                middle += 2;
            } else {
                accumulator -= data[outer][middle];
                middle += 1;
                
                // Inner loop - Level 3 (not fully contained in middle)
                // This for loop is inside an else branch
                for (int inner = 0; inner < CHUNK; inner++) {
                    // Multiple basic blocks in inner loop
                    if (inner % 3 == 0) {
                        data[outer][middle] += inner;
                        if (g_inner_skip) {
                            break;  // Another basic block
                        }
                    } else if (inner % 3 == 1) {
                        data[outer][middle] *= inner;
                        continue;  // Creates separate basic block
                    } else {
                        // goto creates additional control flow
                        if (inner == CHUNK/2) {
                            goto skip_point;
                        }
                        data[outer][middle] ^= inner;
                    }
                    
                    // Label for goto
                    skip_point:
                    accumulator += data[outer][middle] & 0xFF;
                }
            }
            
            // Do-while creates different loop structure
            int post = 0;
            do {
                accumulator ^= data[outer][middle] >> post;
                post++;
            } while (post < 4 && loop_control);
        }
        
        // Another conditional with nested loop (partial overlap)
        if (outer % 4 == 0) {
            // This loop shares some blocks with outer but not all
            for (int alt = SIZE-1; alt >= 0; alt -= 2) {
                if (alt % 3 == 0) {
                    data[outer][alt] = accumulator % 256;
                } else {
                    data[outer][alt] = accumulator / 256;
                }
                
                // Nested switch for more basic blocks
                switch (alt % 4) {
                    case 0:
                        accumulator += alt;
                        break;
                    case 1:
                        accumulator -= alt;
                        break;
                    case 2:
                        accumulator |= alt;
                        // Fall through
                    default:
                        accumulator &= ~alt;
                }
            }
        }
    }
    
    // Second set of nested loops with different overlap pattern
    volatile int row = 0;
    volatile int col = 0;
    
    while (row < SIZE/2) {
        // Loop with variable bounds
        int limit = (row % 2) ? SIZE : SIZE/2;
        
        for (col = row; col < limit; col++) {
            // Complex condition with side effects
            if ((data[row][col] % 2) && (g_volatile_counter++ % 3)) {
                // Deeply nested in condition
                for (int depth = 0; depth < 5; depth++) {
                    if (depth % 2) {
                        data[row][col] += depth * row;
                    } else {
                        data[row][col] -= depth * col;
                        
                        // Innermost with break/continue
                        for (int innermost = 0; innermost < 3; innermost++) {
                            if (innermost == 1) continue;
                            accumulator += data[row][col] >> innermost;
                            if (accumulator > 1000) break;
                        }
                    }
                }
            }
        }
        
        // Mixed loop type
        do {
            row++;
            accumulator += row * col;
        } while (row < SIZE/2 && accumulator < 10000);
    }
    
    // Final computation to prevent elimination
    int result = 0;
    for (int i = 0; i < SIZE; i += 10) {
        for (int j = 0; j < SIZE; j += 10) {
            result += data[i][j] % 100;
        }
    }
    
    printf("Result: %d (Accumulator: %d)\n", result, accumulator);
    return result != 0 ? 0 : 1;
}

#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_SIZE 50

volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;
volatile int g_middle_skip = 1;
volatile int g_inner_skip = 0;

int main() {
    int array[SIZE][SIZE];
    int result = 0;
    
    // Initialize array with pattern
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = (i * 3 + j * 7) % 19;
        }
    }
    
    // Complex nested loop structure with partial overlaps
    // Outer loop - level 1
    for (int outer = 0; outer < SIZE; outer += 2) {
        g_volatile_counter++;
        
        // First basic block in outer loop
        if (g_outer_skip > 0) {
            // Skip path - creates separate basic block
            result += array[outer][0];
            continue;  // Creates another basic block
        }
        
        // Conditional that sometimes skips middle loop entirely
        if (outer % 3 != 0) {
            // Middle loop - level 2 (not fully contained in outer)
            int middle = 0;
            while (middle < INNER_SIZE) {
                // Multiple basic blocks in middle loop
                if (middle % 4 == 0) {
                    // Branch creates separate basic block
                    result += array[outer][middle] * 2;
                    g_middle_skip = middle % 7;
                } else {
                    result -= array[outer][middle];
                    if (g_middle_skip == 0) {
                        // Early continue creates another basic block
                        middle += 2;
                        continue;
                    }
                }
                
                // Inner loop - level 3 (not fully contained in middle)
                // Only execute inner loop in some iterations
                if (middle % 5 != 0) {
                    // for loop with constant bounds
                    for (int inner = 0; inner < middle + 5; inner++) {
                        // Complex inner loop body with multiple blocks
                        volatile int local_volatile = inner % 3;
                        
                        if (local_volatile == 0) {
                            result += array[inner][outer] * 3;
                            if (g_inner_skip > 0) {
                                // Break creates separate exit block
                                break;
                            }
                        } else if (local_volatile == 1) {
                            result -= array[inner][middle];
                            // goto creates another basic block
                            if (inner % 11 == 0) goto inner_skip;
                        } else {
                            result ^= array[middle][inner];
                        }
                        
                        // Label for goto target
                        inner_skip:
                        g_volatile_counter += inner % 2;
                    }
                }
                
                // Do-while loop inside middle loop (different type)
                int dw_counter = 0;
                do {
                    if (dw_counter % 2 == 0) {
                        result += array[dw_counter][middle];
                    } else {
                        result -= array[middle][dw_counter];
                        if (dw_counter % 7 == 0) break;
                    }
                    dw_counter++;
                } while (dw_counter < 3);
                
                middle += 1 + (result % 2);  // Variable increment
            }
        } else {
            // Alternative path in outer loop (no middle loop)
            // This creates blocks in outer that aren't in middle
            for (int alt = 0; alt < 10; alt++) {
                result += array[outer][alt] * alt;
            }
        }
        
        // Final block in outer loop (after middle loop region)
        result = (result * 13) % 10007;
    }
    
    // Additional loop structure with different overlap pattern
    volatile int control = SIZE / 2;
    int second_outer = 0;
    
    while (second_outer < control) {
        // This loop partially overlaps with previous structure
        // through shared array access patterns
        
        for (int partial_inner = second_outer; 
             partial_inner < second_outer + 20; 
             partial_inner++) {
            if (partial_inner % 2 == 0) {
                // Access creates intersection with previous loops
                result += array[second_outer][partial_inner % SIZE];
            } else {
                // Different block not in previous loops
                result -= partial_inner * second_outer;
            }
        }
        
        // Nested loop that sometimes executes
        if (second_outer % 4 == 0) {
            int k = 0;
            do {
                result ^= array[k][second_outer];
                k++;
                if (k > 15) {
                    // Break to outer scope
                    goto outer_break;
                }
            } while (k < 10);
        }
        
        outer_break:
        second_outer += 1 + (g_volatile_counter % 3);
    }
    
    printf("Result: %d\n", result);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return result % 255;
}

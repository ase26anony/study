#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_SIZE 50

volatile int g_outer_cond = 1;
volatile int g_middle_cond = 1;
volatile int g_inner_cond = 1;
volatile int g_counter = 0;
volatile int g_prevent_opt = 0;

int main() {
    int array[SIZE][SIZE];
    int result = 0;
    
    // Initialize array with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * j + (i ^ j);
        }
    }
    
    // Complex nested loop structure with partial overlaps
    // Outer loop - level 1
    for (int i = 0; i < SIZE; i += 2) {
        // First basic block in outer loop
        g_counter++;
        
        // Conditional that creates partial overlap
        if (g_outer_cond || (i % 3 == 0)) {
            // Middle loop - level 2 (not fully contained in outer)
            // This loop starts here but continues outside the if
            int j = 0;
            
            // Loop header block for middle loop
            while (j < INNER_SIZE) {
                // Middle loop body block 1
                result += array[i][j];
                
                // Nested conditional inside middle loop
                if (g_middle_cond && (j % 4 == 0)) {
                    // Inner loop - level 3 (not fully contained in middle)
                    // This creates partial overlap with middle loop
                    for (int k = 0; k < j + 5; k++) {
                        // Inner loop body with multiple blocks
                        if (g_inner_cond || (k % 2 == 0)) {
                            result -= array[k][j];
                            g_prevent_opt ^= result;
                            
                            // Additional block with break possibility
                            if (k > 20 && g_inner_cond) {
                                result >>= 1;
                                // Early exit from inner loop
                                if (result < 0) break;
                            }
                        } else {
                            result += k * j;
                            // Continue to next iteration
                            continue;
                        }
                        
                        // Another block in inner loop
                        g_counter += k;
                    }
                    // End of inner loop
                }
                
                // Middle loop continuation block (outside inner loop's if)
                j += (g_middle_cond ? 1 : 2);
                
                // Another conditional in middle loop
                if (j > INNER_SIZE / 2) {
                    result ^= array[i][j % SIZE];
                    // This creates a block that's in middle but not in inner
                }
            }
            // End of middle loop
        } else {
            // Alternative path in outer loop (creates blocks in outer but not in middle)
            result += i * i;
            // This block is in outer but not in middle loop
        }
        
        // Final block of outer loop (executed regardless of if condition)
        // This creates intersection but not subset relationship
        g_prevent_opt += result & 0xFF;
    }
    
    // Additional loop structure to create more complex bitmap relationships
    // This creates loops that intersect but aren't subsets
    int x = 0;
    do {
        // Do-while loop body
        int y = 0;
        
        // Nested for loop inside do-while
        for (y = 0; y < 10; y++) {
            // Conditional that sometimes skips the innermost loop
            if (x % 2 == 0 || g_outer_cond) {
                int z = 0;
                // While loop inside for loop
                while (z < 5) {
                    // This creates another level of partial overlap
                    result += x * y * z;
                    z += (g_inner_cond ? 1 : 2);
                    
                    // Label for potential goto (creates additional basic blocks)
                    if (z == 3) {
                        result >>= 2;
                        // goto skip_point; // Uncomment for even more complexity
                    }
                    
                    skip_point:
                    g_counter++;
                }
            }
            
            // Block in for loop but not in while loop
            result ^= y;
        }
        
        x++;
        // This block is in do-while but not in for loop
        if (x > 5 && g_middle_cond) {
            result += 1000;
            // Early exit
            if (result > 10000) break;
        }
    } while (x < 15);
    
    // Final computation to prevent dead code elimination
    printf("Result: %d\n", result);
    printf("Counter: %d\n", g_counter);
    printf("Prevent opt: %d\n", g_prevent_opt);
    
    return result != 0 ? 0 : 1;
}

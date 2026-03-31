#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_SIZE 50

volatile int g_outer_cond = 1;
volatile int g_middle_cond = 1;
volatile int g_inner_cond = 1;
volatile int g_accumulator = 0;
volatile int g_control = 0;

int main() {
    int array[SIZE][SIZE];
    int partial_array[INNER_SIZE][INNER_SIZE];
    volatile int result = 0;
    
    // Initialize arrays with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * j + 1;
        }
    }
    
    for (int i = 0; i < INNER_SIZE; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            partial_array[i][j] = (i + j) % 10;
        }
    }
    
    // ====== COMPLEX NESTED LOOP STRUCTURE ======
    // Outer loop with multiple basic blocks
    for (int outer = 0; outer < SIZE; outer += 2) {
        // First basic block in outer loop
        g_accumulator += outer;
        
        // Conditional that creates partial overlap
        if (g_outer_cond || (outer % 3 == 0)) {
            // This block is sometimes skipped
            volatile int temp = outer * 2;
            
            // ====== MIDDLE LOOP (not fully contained) ======
            // Middle loop with its own complex body
            int middle = 0;
            while (middle < SIZE) {
                // Multiple basic blocks in middle loop
                if (middle % 4 == 0) {
                    // Branch 1
                    array[outer][middle] += g_control;
                    g_accumulator++;
                    
                    // ====== INNER LOOP (not fully contained) ======
                    // Inner loop with volatile condition
                    for (int inner = 0; 
                         inner < INNER_SIZE && g_inner_cond; 
                         inner += (g_control % 3) + 1) {
                        
                        // Complex inner loop body with multiple blocks
                        if (inner % 2 == 0) {
                            // Block A in inner loop
                            partial_array[inner % INNER_SIZE][middle % INNER_SIZE] 
                                += array[outer][middle];
                            result ^= partial_array[inner % INNER_SIZE][middle % INNER_SIZE];
                            
                            // Nested if inside inner loop
                            if (g_inner_cond && (result % 7 == 0)) {
                                g_accumulator += 2;
                                // Early continue creates another block
                                continue;
                            }
                        } else {
                            // Block B in inner loop
                            volatile int inner_temp = inner * 3;
                            result += inner_temp;
                            
                            // Break condition that creates exit block
                            if (inner_temp > 100 && g_inner_cond) {
                                g_accumulator -= 1;
                                break;
                            }
                        }
                        
                        // Third block in inner loop
                        g_control = (g_control + 1) % 5;
                    } // End inner for loop
                    
                } else if (middle % 4 == 1) {
                    // Alternative branch that doesn't contain inner loop
                    // This creates partial overlap between middle and outer
                    array[outer][middle] -= g_accumulator;
                    result |= array[outer][middle];
                    
                    // goto label creating another basic block
                    alternate_path:
                    g_control = (g_control * 2) % 7;
                } else {
                    // Third branch with do-while loop (different loop type)
                    int counter = 0;
                    do {
                        array[outer][middle] *= 2;
                        counter++;
                        if (counter > 5) {
                            // goto to create cross-block flow
                            goto alternate_path;
                        }
                    } while (counter < 3 && g_middle_cond);
                }
                
                // Common block after conditional
                middle += (g_control % 2) + 1;
                
                // Continue condition check
                if (middle > SIZE / 2 && g_middle_cond) {
                    // Early exit from middle loop
                    // This creates blocks in middle that aren't in outer
                    g_accumulator += 100;
                    break;
                }
            } // End middle while loop
            
        } else {
            // Alternative outer loop path without middle loop
            // This creates blocks in outer that aren't in middle
            for (int k = 0; k < 10; k++) {
                result += k * outer;
                if (k == 5 && g_outer_cond) {
                    // Nested break
                    break;
                }
            }
        }
        
        // Final block in outer loop
        if (outer % 10 == 0) {
            g_control = (g_control + outer) % 11;
        }
    } // End outer for loop
    
    // ====== ADDITIONAL NESTING PATTERN ======
    // Another set of loops with different overlap pattern
    volatile int x = 0, y = 0, z = 0;
    
    // Triple nested loops with mixed types
    for (x = 0; x < 20; x++) {
        // Outer loop block A
        result += x;
        
        if (x % 2 == g_outer_cond) {
            // Middle do-while loop
            y = 0;
            do {
                // Middle loop block A
                result -= y;
                
                if (y % 3 == 0) {
                    // Inner while loop
                    z = 0;
                    while (z < 15 && g_inner_cond) {
                        // Complex inner body
                        result ^= (x * y * z);
                        z += (g_control % 3) + 1;
                        
                        if (z > 10) {
                            // This block is in inner but not in middle
                            g_accumulator += z;
                            continue;
                        }
                    }
                } else {
                    // This block is in middle but not in inner
                    result |= (x << y);
                }
                
                y++;
                // Middle loop block B (not in inner when inner didn't execute)
                if (y > 10 && g_middle_cond) {
                    result += 1000;
                }
            } while (y < 15);
        } else {
            // This block is in outer but not in middle
            result *= 2;
        }
        
        // Outer loop block B (not in middle when middle didn't execute)
        if (x == 19) {
            g_accumulator = result % 1000;
        }
    }
    
    // Prevent dead code elimination
    printf("Result: %d, Accumulator: %d, Control: %d\n", 
           result, g_accumulator, g_control);
    
    return (result + g_accumulator + g_control) % 256;
}

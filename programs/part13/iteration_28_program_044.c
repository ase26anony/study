#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define HALF_SIZE (SIZE / 2)

volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;
volatile int g_inner_condition = 1;

int main() {
    int array[SIZE][SIZE];
    int result = 0;
    
    // Initialize array with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * SIZE + j;
        }
    }
    
    // Outer loop - Level 1
    for (int outer = 0; outer < SIZE; outer++) {
        // This if statement creates partial overlap
        if (outer % 3 != 0 || g_outer_skip) {
            // Middle loop - Level 2 (not strictly contained in outer)
            // This loop has blocks outside the outer loop's body
            int middle = 0;
            do {
                // Multiple basic blocks in middle loop body
                if (middle % 2 == 0) {
                    // First branch of middle loop
                    result += array[outer][middle];
                    
                    // Inner loop - Level 3 (not strictly contained in middle)
                    // This loop sometimes executes based on volatile
                    for (int inner = 0; inner < HALF_SIZE && g_inner_condition; inner++) {
                        // Complex inner loop body with multiple blocks
                        if (inner % 3 == 0) {
                            result -= array[middle][inner];
                            // Label for potential goto (creates extra basic block)
                            if (inner == HALF_SIZE - 1) {
                                result += 1000;
                            }
                        } else if (inner % 5 == 0) {
                            // Another branch
                            result += array[inner][outer] * 2;
                            continue;  // Creates additional control flow
                        } else {
                            result += inner;
                        }
                        
                        // Volatile access prevents optimization
                        g_volatile_counter++;
                        
                        // Early break creates another basic block
                        if (result > 1000000) {
                            break;
                        }
                    }
                } else {
                    // Second branch of middle loop (different basic blocks)
                    result *= 2;
                    if (result < 0) {
                        result = 0;
                    }
                }
                
                // Update volatile to affect loop conditions
                g_volatile_counter += middle;
                middle++;
                
                // Complex while condition with volatile
            } while (middle < (SIZE - outer % 10) && g_volatile_counter < 10000);
            
            // Code after do-while but still in outer's if branch
            result += outer * 100;
        } else {
            // Else branch of outer loop (different path)
            // Another loop here creates more complex bitmap relationships
            for (int alt = 0; alt < 5; alt++) {
                result += alt * alt;
                if (alt == 3) {
                    // Nested if creates another basic block
                    result -= 50;
                }
            }
        }
        
        // Additional control flow in outer loop
        switch (outer % 4) {
            case 0:
                result += 1;
                break;
            case 1:
                result += 2;
                // Fall through
            case 2:
                result += 3;
                break;
            default:
                result += 4;
                break;
        }
        
        // Modify volatile to affect inner loops
        if (outer % 7 == 0) {
            g_inner_condition = !g_inner_condition;
        }
    }
    
    // Additional loop structure to create more bitmap relationships
    int counter = 0;
    while (counter < 20) {
        // Nested for inside while
        for (int i = 0; i < 10; i++) {
            result += i * counter;
            // Another level of nesting
            int j = 0;
            while (j < 5) {
                result -= j;
                j++;
                if (j == 3 && counter == 10) {
                    goto special_label;  // Creates additional basic block
                }
            }
        }
        counter++;
        
        if (counter == 15) {
            // Break from while creates another exit block
            break;
        }
    }
    
special_label:
    result += 999;
    
    // Final computation to prevent dead code elimination
    printf("Result: %d\n", result);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return result % 256;
}

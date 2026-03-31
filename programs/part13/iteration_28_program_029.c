#include <stdio.h>
#include <stdlib.h>

// Volatile variables to prevent optimization
volatile int g_outer_cond = 1;
volatile int g_middle_cond = 1;
volatile int g_inner_cond = 1;
volatile int g_counter = 0;
volatile int g_modifier = 7;

// Function to create complex control flow within loops
int complex_condition(int x, int y) {
    volatile int a = x * 3;
    volatile int b = y * 5;
    return (a + b) % g_modifier;
}

int main() {
    const int SIZE = 100;
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
    
    // ========== COMPLEX NESTED LOOPS WITH PARTIAL OVERLAP ==========
    
    // Outer loop - Level 1
    for (int outer = 0; outer < SIZE; outer += 2) {
        // Multiple basic blocks in outer loop body
        if (complex_condition(outer, g_counter) > 3) {
            // This block sometimes executes, sometimes not
            accumulator += array1[outer][0];
            
            // Middle loop - Level 2 (NOT strictly contained in outer)
            // Starts in one branch but continues outside
            int middle = 0;
            while (middle < SIZE) {
                // Multiple basic blocks in middle loop
                if (g_middle_cond && (middle % 4 == 0)) {
                    // Skip some iterations
                    middle += 2;
                    continue;
                }
                
                // Inner loop - Level 3 (NOT strictly contained in middle)
                // Only executes in some middle loop iterations
                if (complex_condition(outer, middle) % 3 != 0) {
                    // Fixed iteration count but with internal control flow
                    for (int inner = 0; inner < 50; inner++) {
                        // Complex body with multiple basic blocks
                        if (inner % 2 == 0) {
                            accumulator += array1[outer][inner];
                            if (accumulator > 1000) {
                                // Early exit from inner loop
                                accumulator -= 500;
                                break;
                            }
                        } else {
                            accumulator -= array2[middle][inner];
                        }
                        
                        // Another basic block
                        g_counter++;
                        
                        // Label and goto to create additional blocks
                        if (inner == 25) {
                            goto special_case;
                        }
                        continue;
                        
                    special_case:
                        accumulator *= 2;
                    }
                } else {
                    // Alternative path in middle loop (no inner loop)
                    accumulator += array2[outer][middle];
                }
                
                middle++;
                
                // Middle loop has exit condition that depends on volatile
                if (g_middle_cond && accumulator > 2000) {
                    break;
                }
            }
            
            // More code in outer loop's if branch
            accumulator /= 2;
        } else {
            // Alternative outer loop path (no middle loop)
            for (int k = 0; k < 10; k++) {
                accumulator += k * outer;
            }
        }
        
        // Common outer loop tail (shared by both if/else branches)
        g_counter %= 100;
        
        // Do-while loop inside outer (different type)
        int dw = 0;
        do {
            accumulator += dw;
            dw++;
            if (dw > 5) break;
        } while (g_outer_cond && dw < 10);
    }
    
    // ========== ADDITIONAL NESTING PATTERN ==========
    
    // Another set of nested loops with different overlap pattern
    volatile int x = 0, y = 0, z = 0;
    
    // Outer loop with variable bounds
    for (x = 0; x < 20 + (g_counter % 5); x++) {
        // Conditional that splits the loop body
        if (x % 3 == 0) {
            // Middle loop that starts here but...
            for (y = x; y < 15; y++) {
                // Inner loop with complex exit
                z = 0;
                while (z < 10) {
                    accumulator += x * y * z;
                    z++;
                    if (z == 5 && g_inner_cond) {
                        // Jump to label creating another basic block
                        goto skip_rest;
                    }
                }
                continue;
                
            skip_rest:
                accumulator -= 100;
            }
            // ... continues here (middle loop not strictly in outer's if)
            accumulator += y * 10;
        }
        
        // More outer loop code that middle loop doesn't have
        if (x % 4 == 0) {
            accumulator += 999;
        }
    }
    
    // ========== FINAL OUTPUT ==========
    
    printf("Result: %d\n", accumulator);
    printf("Counter: %d\n", g_counter);
    
    // Use result to prevent dead code elimination
    return accumulator > 0 ? 0 : 1;
}

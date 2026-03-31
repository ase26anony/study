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
    
    // Outer loop - Level 1
    for (int i = 0; i < SIZE; i++) {
        // This if creates a basic block that's in outer but not in middle
        if (g_outer_cond) {
            result += array[i][0];
            g_counter++;
        }
        
        // Middle loop - Level 2 (not strictly contained in outer)
        // This loop starts in one branch but continues in another
        int j = 0;
        while (j < SIZE) {
            // Basic block that's in middle but may not be in inner
            if (j % 3 == 0) {
                result -= array[i][j];
                g_prevent_opt ^= result;
            }
            
            // Inner loop - Level 3 (not strictly contained in middle)
            // This creates partial overlap with middle loop
            if (g_middle_cond && j > 10 && j < SIZE - 10) {
                for (int k = 0; k < INNER_SIZE; k++) {
                    // Complex inner loop body with multiple basic blocks
                    if (k % 2 == 0) {
                        array[i][j] += k;
                        if (g_inner_cond) {
                            result ^= array[i][j];
                            // This goto creates additional control flow
                            if (result < 0) goto skip_point;
                        }
                    } else {
                        array[i][j] -= k;
                        // Continue statement creates another basic block
                        if (array[i][j] % 7 == 0) continue;
                    }
                    
                    // Another basic block in inner loop
                    g_counter += k;
                    if (g_counter > 1000) {
                        g_counter = 0;
                        break;  // Early exit creates more complexity
                    }
                    
                    skip_point:
                    // Empty label for goto target
                    g_prevent_opt++;
                }
            }
            
            // Another basic block in middle loop after inner loop
            j += (g_prevent_opt % 2) + 1;
            
            // Do-while creates different loop structure
            do {
                result += j;
                if (j % 5 == 0) break;
                j++;
            } while (j < SIZE && g_middle_cond);
        }
        
        // Final basic block in outer loop
        if (i % 7 == 0) {
            for (int x = 0; x < 5; x++) {
                result += x * i;
            }
        }
    }
    
    // Additional nested loop structure with different pattern
    volatile int a = 0, b = 0, c = 0;
    
    // Triple nested loops with complex conditions
    for (a = 0; a < 20; a++) {
        // Conditional that sometimes skips the inner loops
        if (a % 3 != 0) {
            b = 0;
            while (b < 15) {
                // Loop with variable increment
                c = b;
                do {
                    result += array[a % SIZE][c % SIZE];
                    c++;
                    if (c % 4 == 0) {
                        // Nested if creates more basic blocks
                        result >>= 1;
                        goto inner_label;
                    }
                } while (c < 20);
                
                inner_label:
                b += (result % 3) + 1;
                
                // Another loop inside while but not do-while
                for (int d = 0; d < 3; d++) {
                    result ^= d * b;
                    if (result > 10000) result = 0;
                }
            }
        } else {
            // Alternative path in outer loop
            result -= a * 7;
        }
    }
    
    // Print result to prevent optimization
    printf("Result: %d\n", result);
    printf("Counter: %d\n", g_counter);
    printf("PreventOpt: %d\n", g_prevent_opt);
    
    return result != 0 ? 0 : 1;
}

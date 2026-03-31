#include <stdio.h>
#include <stdlib.h>

#define SIZE 64
#define HALF_SIZE (SIZE / 2)

volatile int g_outer_cond = 1;
volatile int g_middle_cond = 0;
volatile int g_inner_cond = 1;
volatile int g_skip_inner = 0;
volatile int g_break_early = 0;
volatile int g_continue_flag = 0;

volatile long long accumulator = 0;

int main() {
    int array[SIZE][SIZE];
    int temp_array[SIZE];
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * SIZE + j;
        }
        temp_array[i] = 0;
    }
    
    // Complex nested loop structure with partial overlaps
    // Outer loop - Level 1
    for (int outer = 0; outer < SIZE; outer += 2) {
        // Multiple basic blocks in outer loop body
        if (g_outer_cond) {
            // This block is in outer but not in middle/inner
            
            // Middle loop - Level 2 (not strictly contained in outer)
            // Starts in one branch, continues in another
            int middle_start = outer % 4;
            int middle_end = SIZE - (outer % 3);
            
            // First part of middle loop control flow
            if (middle_start < middle_end) {
                int j = middle_start;
                
                // Use while for middle loop (mixed loop type)
                while (j < middle_end) {
                    // Multiple basic blocks in middle loop body
                    if (g_middle_cond) {
                        // Block in middle but not in inner
                        temp_array[j] += array[outer][j];
                        
                        // Inner loop - Level 3 (not strictly contained in middle)
                        // Only executes sometimes
                        if (!g_skip_inner && g_inner_cond) {
                            // Fixed iteration for loop (encourages hardware loop)
                            for (int k = 0; k < HALF_SIZE; k++) {
                                // Complex inner loop body with multiple blocks
                                if (k % 3 == 0) {
                                    array[outer][j] += k;
                                    accumulator += array[outer][j];
                                    
                                    // Early break possibility
                                    if (g_break_early && k > 10) {
                                        break;
                                    }
                                    
                                    // Continue with label
                                    if (g_continue_flag) {
                                        continue;
                                    }
                                } else if (k % 5 == 0) {
                                    // Another basic block in inner loop
                                    array[outer][j] -= k;
                                    accumulator -= array[outer][j];
                                    
                                    // Goto within loop body
                                    if (k == 15) {
                                        goto special_case;
                                    }
                                } else {
                                    // Third basic block
                                    array[outer][j] *= 2;
                                    accumulator += array[outer][j] * 3;
                                }
                                
                                special_case:
                                // Empty label for goto target
                                if (k == 20) {
                                    // Do something special
                                    accumulator += 1000;
                                }
                            }
                        } else {
                            // Alternative path in middle loop (not containing inner)
                            temp_array[j] *= 2;
                            accumulator += temp_array[j];
                        }
                    } else {
                        // Another alternative path
                        temp_array[j] -= array[outer][j];
                        accumulator -= temp_array[j];
                    }
                    
                    // Middle loop increment with condition
                    j += 1 + (j % 2);
                    
                    // Middle loop might skip iterations
                    if (j % 7 == 0) {
                        j += 2;
                    }
                }
            }
            
            // Outer loop continues here (block not in middle/inner)
            accumulator += outer * 100;
        } else {
            // Alternative outer loop path (completely skips middle/inner)
            for (int alt = 0; alt < HALF_SIZE; alt++) {
                temp_array[alt] = alt * alt;
                accumulator += temp_array[alt];
            }
        }
        
        // Another middle loop instance in outer loop (different structure)
        // This creates partial overlap with first middle loop
        if (outer % 3 == 0) {
            int m = 0;
            // do-while loop (another mixed type)
            do {
                // This block overlaps with some but not all of first middle loop
                array[outer][m] += temp_array[m];
                
                // Nested loop with goto creating complex CFG
                for (int n = 0; n < 8; n++) {
                    if (n == 4) {
                        goto skip_point;
                    }
                    accumulator += array[outer][m] * n;
                    
                    skip_point:
                    // Label for goto
                    if (n == 6) {
                        break;
                    }
                }
                
                m++;
            } while (m < HALF_SIZE && g_inner_cond);
        }
    }
    
    // Additional loop nest with different overlap pattern
    // Triple nested loops with complex conditions
    volatile int control = SIZE / 4;
    
    for (int a = 0; a < control; a++) {
        // Loop with switch inside
        switch (a % 3) {
            case 0: {
                // Loop with variable bounds
                int b_limit = control + (a % 5);
                for (int b = a; b < b_limit; b++) {
                    // Inner loop that sometimes executes
                    if (b % 2 == 0) {
                        int c = 0;
                        while (c < 10) {
                            // Complex body with multiple exits
                            if (c == 5) {
                                goto exit_inner;
                            }
                            accumulator += array[a][b] * c;
                            c++;
                        }
                        exit_inner:
                        // Target for goto
                        accumulator += 1;
                    }
                }
                break;
            }
            case 1:
                // Different structure - no inner loop here
                accumulator -= array[a][0];
                break;
            case 2:
                // Yet another structure
                for (int d = 0; d < 5; d++) {
                    accumulator += d * d;
                }
                break;
        }
    }
    
    // Final computation to prevent elimination
    printf("Result: %lld\n", accumulator);
    
    // Use result to prevent dead code elimination
    if (accumulator > 1000000) {
        return 0;
    } else {
        return 1;
    }
}

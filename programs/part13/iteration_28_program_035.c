#include <stdio.h>
#include <stdlib.h>

// Volatile variables to prevent optimization
volatile int g_volatile_counter = 0;
volatile int g_outer_cond = 1;
volatile int g_middle_cond = 1;
volatile int g_inner_cond = 1;

// Function to create complex control flow within loops
int complex_condition(int x, int y) {
    volatile int v = x * y;
    if (v % 3 == 0) return 1;
    if (v % 5 == 0) return 2;
    if (v % 7 == 0) return 3;
    return 0;
}

int main() {
    // Multi-dimensional array for loop computations
    int array[10][10][10];
    volatile int accumulator = 0;
    
    // Initialize array with non-zero values
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                array[i][j][k] = i * 100 + j * 10 + k + 1;
            }
        }
    }
    
    // ========== COMPLEX NESTED LOOP STRUCTURE ==========
    // Outer loop - Level 1
    for (int outer = 0; outer < 8; outer++) {
        // Create multiple basic blocks in outer loop body
        if (g_outer_cond && outer % 2 == 0) {
            // This block sometimes executes, sometimes not
            accumulator += outer * 10;
            
            // Middle loop - Level 2 (NOT strictly contained in outer)
            // This loop starts in one branch but may continue in another
            int middle_start = (outer % 3) + 1;
            int middle_end = 7 - (outer % 2);
            
            // First basic block of middle loop
            volatile int middle_init = middle_start;
            
            // The middle loop body has multiple exit points
            while (middle_init <= middle_end) {
                // Multiple basic blocks within middle loop
                if (complex_condition(outer, middle_init) == 1) {
                    // Skip some iterations
                    middle_init += 2;
                    continue;
                }
                
                // Inner loop - Level 3 (NOT strictly contained in middle)
                // This loop has complex entry conditions
                if (g_middle_cond || (outer + middle_init) % 4 != 0) {
                    // do-while loop for variety
                    int inner = 0;
                    int inner_limit = 5 - (middle_init % 3);
                    
                    // Label for goto to create additional basic block
                    inner_loop_start:
                    do {
                        // Multiple basic blocks in inner loop
                        if (g_inner_cond && inner % 2 == 0) {
                            // Computation that uses all loop indices
                            array[outer][middle_init][inner] += 
                                accumulator + outer + middle_init + inner;
                            
                            // Conditional break
                            if (inner > 2 && (inner * outer) % 5 == 0) {
                                break;
                            }
                        } else {
                            // Alternative path
                            accumulator -= inner;
                            
                            // goto to create additional control flow
                            if (inner == 1 && outer > 3) {
                                goto skip_inner_update;
                            }
                        }
                        
                        // Update array in different way
                        array[outer][middle_init][inner] *= 2;
                        
                        skip_inner_update:
                        inner++;
                        
                        // Complex loop condition with volatile
                        if (inner == inner_limit && g_volatile_counter > 0) {
                            // Early exit based on volatile
                            goto middle_loop_continuation;
                        }
                    } while (inner < inner_limit);
                    
                    // After inner loop, sometimes skip to middle loop update
                    if (middle_init % 2 == 0) {
                        goto middle_update;
                    }
                }
                
                middle_loop_continuation:
                // Another basic block in middle loop
                accumulator += array[outer][middle_init][0];
                
                middle_update:
                middle_init++;
                
                // Conditional continue based on volatile
                if (g_volatile_counter > 100 && middle_init > middle_end / 2) {
                    break;
                }
            }
            
            // Additional basic block after middle loop
            if (outer % 3 == 0) {
                g_volatile_counter++;
            }
        } else {
            // Alternative path in outer loop (no middle loop here)
            // This creates partial overlap - some outer blocks don't contain middle
            for (int alt = 0; alt < 3; alt++) {
                accumulator -= alt;
                if (alt == 1) {
                    // Nested loop in alternative path (different structure)
                    int temp = 0;
                    while (temp < 2) {
                        array[outer][0][temp] = temp * outer;
                        temp++;
                    }
                }
            }
        }
        
        // Final basic block of outer loop
        if (accumulator > 1000) {
            accumulator = accumulator % 1000;
        }
    }
    
    // ========== SECOND NESTING STRUCTURE ==========
    // Different loop types and nesting pattern
    volatile int control = 5;
    
    // while loop as outer
    int w_outer = 0;
    while (w_outer < control) {
        // Fixed-count for loop as middle
        for (int w_middle = w_outer; w_middle < 6; w_middle += 1 + (w_outer % 2)) {
            // This creates partial overlap - middle doesn't start at w_outer=0 always
            
            // do-while as inner with complex exit
            int w_inner = 0;
            do {
                // Multiple blocks with gotos
                if (w_inner == 0) {
                    goto inner_compute;
                }
                
                array[w_outer][w_middle][w_inner] += w_inner;
                goto inner_next;
                
                inner_compute:
                array[w_outer][w_middle][w_inner] = w_outer * w_middle;
                
                inner_next:
                w_inner++;
                
                // Volatile condition affects loop structure
                if (g_volatile_counter > 50 && w_inner > 2) {
                    break;
                }
            } while (w_inner < 4 - (w_middle % 2));
            
            // Middle loop has block that sometimes executes
            if (w_middle % 3 == 0) {
                accumulator += array[w_outer][w_middle][0];
                // Early exit from middle based on volatile
                if (g_volatile_counter > 75) {
                    goto outer_loop_update;
                }
            }
        }
        
        outer_loop_update:
        w_outer++;
        control += (accumulator % 2); // Volatile-like change
    }
    
    // ========== THIRD NESTING STRUCTURE ==========
    // Triple nested for loops with complex internal conditions
    for (int i = 0; i < 5; i++) {
        // First basic block
        volatile int start_j = (i % 2) * 2;
        
        // Middle loop with variable bounds
        for (int j = start_j; j < 6; j++) {
            // Conditional that splits the loop body
            if (j % 2 == 0) {
                // Inner loop in one branch
                for (int k = 0; k < 3; k++) {
                    array[i][j][k] = i + j + k;
                    if (k == 1 && g_volatile_counter > 10) {
                        // Early continue to outer loop
                        goto next_i_iteration;
                    }
                }
            } else {
                // Different inner loop structure in other branch
                int k = 0;
                while (k < 2) {
                    array[i][j][k] = i * j * k;
                    k++;
                    // This creates different basic blocks
                    if (k == 1) continue;
                }
            }
            
            // Common block after the if-else
            accumulator += array[i][j][0];
        }
        
        next_i_iteration:
        // Outer loop continuation
        if (i % 3 == 0) {
            g_volatile_counter--;
        }
    }
    
    // Use results to prevent dead code elimination
    printf("Result: %d (volatile counter: %d)\n", 
           accumulator % 1000, g_volatile_counter);
    
    // Verify some array values were modified
    int check_sum = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                check_sum += array[i][j][k] % 256;
            }
        }
    }
    
    return (check_sum + accumulator) % 255;
}

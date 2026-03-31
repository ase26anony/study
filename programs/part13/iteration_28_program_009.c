#include <stdio.h>
#include <stdlib.h>

// Volatile variables to prevent optimization
volatile int g_volatile_counter = 0;
volatile int g_outer_cond = 1;
volatile int g_inner_cond = 1;

// Function to create complex basic blocks
int complex_operation(int x, int y) {
    volatile int result = 0;
    
    // Multiple basic blocks within a function
    if (x > y) {
        result = x * y;
        if (result > 100) {
            result -= 50;
        } else {
            result += 25;
        }
    } else {
        result = x + y;
        if (result < 0) {
            result = -result;
        }
    }
    
    // Label for potential goto (creates another basic block)
    if (result == 0) {
        goto zero_case;
    }
    
    return result;
    
zero_case:
    return 1;
}

int main() {
    const int SIZE = 100;
    int array[SIZE][SIZE];
    volatile int accumulator = 0;
    
    // Initialize array
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * j + (i + j) % 7;
        }
    }
    
    // ===== COMPLEX NESTED LOOPS WITH PARTIAL OVERLAP =====
    
    // Outer loop - level 1
    for (int i = 0; i < SIZE; i += 2) {  // Fixed iteration count
        volatile int outer_mod = i % 3;
        
        // Multiple basic blocks in outer loop body
        if (outer_mod == 0) {
            accumulator += complex_operation(i, i);
            
            // Middle loop - level 2 (NOT strictly contained in outer)
            // This loop only executes in this branch
            int j = 0;
            while (j < SIZE) {  // While loop for mixed types
                volatile int middle_check = array[i][j] % 5;
                
                // Another conditional creating more basic blocks
                if (middle_check > 2) {
                    // Inner loop - level 3 (NOT strictly contained in middle)
                    for (int k = j; k < SIZE && k < j + 10; k++) {  // Fixed bounds
                        volatile int inner_val = array[i][k];
                        
                        // Complex inner loop body with multiple blocks
                        if (inner_val % 2 == 0) {
                            accumulator += inner_val;
                            if (accumulator > 10000) {
                                accumulator = accumulator % 1000;
                            }
                        } else {
                            accumulator -= inner_val / 2;
                            // Continue statement creates another edge
                            if (inner_val < 0) continue;
                        }
                        
                        // Break possibility
                        if (g_volatile_counter++ > 1000) break;
                    }
                } else {
                    // Alternative path in middle loop (outside inner loop)
                    accumulator -= array[i][j];
                }
                
                j += (middle_check + 1);
                
                // Mixed loop control
                if (j > SIZE / 2 && g_outer_cond) {
                    j += 5;  // Skip ahead
                }
            }
        } else if (outer_mod == 1) {
            // Different branch - middle loop NOT here
            // This creates partial overlap: some outer blocks don't contain middle
            accumulator *= 2;
            if (accumulator < 0) {
                accumulator = 0;
            }
        } else {
            // Another branch with different loop structure
            do {
                accumulator += i;
                if (accumulator > 5000) break;
            } while (g_inner_cond && (accumulator < 10000));
        }
        
        // Common code after conditional (part of outer but not in some middle paths)
        g_volatile_counter = (g_volatile_counter + 1) % 100;
    }
    
    // ===== SECOND SET OF NESTED LOOPS WITH DIFFERENT PATTERN =====
    
    volatile int control = 50;
    
    // Another outer loop
    for (int x = 0; x < control; x++) {
        // Conditional with goto (creates interesting CFG)
        if (x % 4 == 0) {
            goto skip_inner;
        }
        
        // Middle loop that starts here
        int y = x;
        while (y < control) {
            // Inner loop in only some iterations
            if (y % 3 == 0) {
                for (int z = 0; z < 5; z++) {
                    accumulator += x * y * z;
                    // Early exit creates another block
                    if (accumulator > 20000) goto loop_exit;
                }
            }
            y += (x % 2) + 1;
        }
        
    skip_inner:
        // Code that executes when inner is skipped
        accumulator -= x;
        
        // Nested loop that's conditionally executed
        if (x > control / 2) {
            for (int w = 0; w < 3; w++) {
                accumulator += w;
            }
        }
    }
    
loop_exit:
    
    // ===== THIRD PATTERN: INTERLEAVED LOOPS =====
    
    int a = 0, b = 0;
    
    // Do-while as outer
    do {
        // For loop as middle (partially overlapping)
        for (int c = a; c < 20; c += 2) {
            // While as inner (partially overlapping with middle)
            int d = 0;
            while (d < 10) {
                accumulator += array[a % SIZE][d % SIZE];
                d += (c % 3) + 1;
                
                // Complex break condition
                if (accumulator < -1000 || accumulator > 100000) {
                    goto finish;
                }
            }
            
            // Code after inner but still in middle
            if (c % 5 == 0) {
                accumulator = accumulator / 2;
            }
        }
        
        a++;
        b = (b + a) % 7;
        
    } while (a < 15 && b != 0);
    
finish:
    
    // Prevent dead code elimination
    printf("Result: %d\n", accumulator);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return accumulator != 0 ? 0 : 1;
}

#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 20

volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;
volatile int g_inner_skip = 1;

int main() {
    int array[SIZE][SIZE];
    volatile int accumulator = 0;
    volatile int control = 0;
    
    // Initialize array with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * j + (i ^ j);
        }
    }
    
    // Complex nested loop structure with partial overlaps
    // Outer loop - Level 1
    for (int i = 0; i < SIZE; i++) {
        // Multiple basic blocks in outer loop body
        if (i % 3 == 0) {
            // Skip inner loops sometimes
            g_outer_skip = (i % 6 == 0) ? 1 : 0;
            accumulator += i;
            continue;  // Creates additional basic block
        }
        
        // Middle loop - Level 2 (not strictly contained in outer)
        // This loop starts in outer but may exit early
        int j = 0;
        while (j < SIZE) {
            // Multiple basic blocks in middle loop
            if (j % 5 == 0) {
                // Early exit from middle loop
                if (control > 50) {
                    break;  // Creates exit block
                }
                accumulator += array[i][j];
                j += 2;
                continue;
            }
            
            // Inner loop - Level 3 (not strictly contained in middle)
            // Only executes in some iterations
            if (!g_inner_skip || (i + j) % 7 != 0) {
                // Fixed count loop inside conditional
                for (int k = 0; k < CHUNK; k++) {
                    // Complex inner loop body
                    if (k % 2 == 0) {
                        accumulator += array[i][j] * k;
                        if (accumulator > 10000) {
                            // Reset but continue
                            accumulator %= 1000;
                        }
                    } else {
                        accumulator -= array[j][i] / (k + 1);
                    }
                    
                    // Volatile check prevents optimization
                    if (g_volatile_counter++ > 1000) {
                        g_volatile_counter = 0;
                    }
                }
                
                // Another basic block after inner loop
                control += (i * j) % 11;
            }
            
            // Middle loop increment with condition
            j += (j % 3 == 0) ? 2 : 1;
            
            // Label for potential goto (creates another basic block)
            mid_loop_tail:
            if (j > SIZE / 2 && control < 30) {
                // Jump back creates interesting control flow
                goto mid_loop_tail;
            }
        }
        
        // Do-while loop at outer level (mixed loop type)
        int m = 0;
        if (i % 4 == 0) {
            do {
                // This block is in outer but not in middle
                accumulator += m * i;
                m++;
                
                // Nested if-else creates more blocks
                if (m % 2 == 0) {
                    volatile int temp = array[i][m % SIZE];
                    accumulator += temp;
                } else {
                    accumulator -= 1;
                }
            } while (m < 10 && accumulator < 5000);
        }
    }
    
    // Second set of nested loops with different overlap pattern
    volatile int x = 0, y = 0, z = 0;
    
    // Outer loop with goto-based control
    for (x = 0; x < SIZE / 2; x++) {
        start_middle:
        // Middle loop that can be skipped
        if (x % 3 != 0) {
            for (y = x; y < SIZE && y < x + CHUNK; y++) {
                // Inner loop with variable bounds
                int limit = (x + y) % 10 + 5;
                z = 0;
                while (z < limit) {
                    // Multiple blocks in while body
                    if (z % 2 == 0) {
                        array[x][y] += z;
                        if (array[x][y] > 1000) {
                            array[x][y] = 0;
                            goto reset_inner;  // Creates another block
                        }
                    }
                    z++;
                    continue;
                    
                reset_inner:
                    z = limit / 2;
                    continue;
                }
                
                // Conditional continue in middle loop
                if (y % 7 == 0) {
                    y += 3;
                    goto start_middle;  // Partial overlap pattern
                }
            }
        }
        
        // Another inner loop in different branch
        if (x % 4 == 0) {
            for (int w = 0; w < 8; w++) {
                accumulator += w * x;
                // This inner loop shares some blocks with previous
                // but not all, creating partial overlap
            }
        }
    }
    
    // Third nesting pattern with deeply nested conditionals
    volatile int a = 0, b = 0, c = 0;
    
    for (a = 0; a < 50; a += 2) {
        // Complex conditional with loops on both branches
        if (accumulator % 2 == 0) {
            // First branch with loop
            b = a;
            while (b < a + 15) {
                // Inner loop in first branch
                for (c = 0; c < 5; c++) {
                    array[a % SIZE][b % SIZE] += c;
                    if (c == 3) break;
                }
                b += (b % 2 == 0) ? 1 : 2;
            }
        } else {
            // Second branch with different loop structure
            for (b = a; b > 0; b--) {
                // Do-while inside for
                c = 0;
                do {
                    accumulator -= b * c;
                    c++;
                } while (c < 3 && accumulator > -1000);
            }
        }
        
        // Loop that sometimes executes
        if (a % 5 == 0) {
            for (int d = 0; d < 3; d++) {
                // Empty loop body still creates blocks
                ;
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return accumulator != 0 ? 0 : 1;
}

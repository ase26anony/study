#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 25

volatile int g_control = 0;
volatile int g_counter = 0;
volatile int g_accumulator = 0;

int main() {
    // Initialize arrays with volatile elements to prevent optimization
    volatile int data[SIZE][SIZE];
    volatile int temp[SIZE];
    
    // Initialize with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            data[i][j] = (i * j) % 7;
        }
        temp[i] = i % 13;
    }
    
    // Set up control variable with non-trivial pattern
    g_control = 1;
    
    // ====== COMPLEX NESTED LOOP STRUCTURE ======
    // Outer loop - level 1
    for (int i = 0; i < SIZE; i += 2) {
        // Multiple basic blocks in outer loop body
        if (g_control > 0) {
            // First basic block in outer loop
            g_accumulator += data[i][0];
            
            // Middle loop - level 2 (not strictly contained in outer)
            // This loop starts in outer's body but has blocks outside
            int j = i;
            while (j < SIZE && j < i + CHUNK) {
                // Multiple basic blocks in middle loop
                if (temp[j] % 3 == 0) {
                    // Inner loop - level 3 (not strictly contained in middle)
                    // This creates partial overlap scenario
                    for (int k = 0; k < CHUNK; k++) {
                        // Complex inner loop body with multiple blocks
                        volatile int inner_control = data[i][j] + k;
                        
                        if (inner_control % 2 == 0) {
                            g_accumulator += data[i][k];
                            // Additional basic block for even case
                            if (k % 5 == 0) {
                                g_counter++;
                                // Early continue creates another block
                                continue;
                            }
                        } else {
                            g_accumulator -= temp[k];
                            // Break can exit inner loop early
                            if (inner_control > 50) {
                                // Label for potential goto (creates block)
                                early_exit:
                                g_counter += 2;
                                break;
                            }
                        }
                        
                        // Another conditional in inner loop
                        if (g_accumulator > 1000) {
                            goto early_exit;
                        }
                    }
                } else {
                    // Alternative path in middle loop (not containing inner)
                    g_accumulator *= 2;
                    // Do-while loop inside else branch
                    int m = 0;
                    do {
                        temp[m % SIZE] += g_accumulator;
                        m++;
                    } while (m < 5 && g_control > 0);
                }
                
                // Middle loop increment with condition
                j += (temp[j] % 2) + 1;
                
                // Continue or break based on volatile
                if (g_counter > 100) {
                    break;
                }
            }
            
            // Back to outer loop body (different block)
            if (i % 7 == 0) {
                // Another loop in outer but not containing middle
                for (int n = 0; n < 10; n++) {
                    g_accumulator += n * g_control;
                }
            }
        } else {
            // Alternative outer loop path (no inner loops here)
            g_accumulator -= temp[i % SIZE];
        }
        
        // Final block of outer loop
        g_control = (g_control + 1) % 3;
    }
    
    // ====== SECOND NESTING PATTERN ======
    // Another set of loops with different overlap pattern
    volatile int control2 = SIZE / 2;
    
    // Outer do-while
    do {
        // Middle for loop with volatile bound
        for (int x = control2; x > 0; x -= (g_control + 1)) {
            // Inner while with complex condition
            int y = x;
            while (y > 0 && g_accumulator < 5000) {
                // Nested if creating multiple blocks
                if (y % 4 == 0) {
                    g_accumulator += data[y % SIZE][x % SIZE];
                    y -= 3;
                } else if (y % 4 == 1) {
                    g_accumulator -= temp[y % SIZE];
                    y -= 2;
                    continue;  // Skip to while condition
                } else {
                    g_accumulator *= 1.1;
                    y--;
                }
                
                // Another conditional block
                if (g_accumulator < 0) {
                    g_accumulator = 0;
                }
            }
            
            // This block is in middle but not in inner
            if (x % 10 == 0) {
                control2--;
            }
        }
        
        // Block in outer but not in middle
        g_counter += control2;
    } while (control2 > 10 && g_counter < 200);
    
    // ====== THIRD PATTERN WITH GOTO ======
    // Loops with goto creating non-standard control flow
    int a = 0, b = 0;
    
    outer_label:
    for (; a < SIZE; a += 4) {
        // Conditional entry to middle loop
        if (data[a][0] % 2 == g_control) {
            b = a;
            middle_label:
            while (b < a + 20 && b < SIZE) {
                // Inner with early exit
                for (int c = 0; c < 15; c++) {
                    if (data[a][b] + c > 100) {
                        // Jump to middle loop update
                        b += 5;
                        goto middle_label;
                    }
                    g_accumulator += c;
                    
                    if (g_accumulator > 3000) {
                        // Jump to outer loop update
                        a += 8;
                        goto outer_label;
                    }
                }
                b += (temp[b] % 3) + 1;
            }
        }
        
        // Another inner loop in different branch
        if (a % 3 == 0) {
            int d = 0;
            do {
                g_counter += data[a][d % SIZE];
                d++;
            } while (d < 8);
        }
    }
    
    // Prevent dead code elimination
    printf("Result: accumulator = %d, counter = %d\n", 
           g_accumulator, g_counter);
    
    // Use results to affect return value
    return (g_accumulator + g_counter) > 0 ? 0 : 1;
}

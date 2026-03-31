#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 20

volatile int g_volatile_counter = 0;
volatile int g_condition = 1;

int main() {
    int array[SIZE][SIZE];
    volatile int accumulator = 0;
    volatile int outer_skip = 0;
    
    // Initialize array
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * SIZE + j;
        }
    }
    
    // ====== COMPLEX NESTED LOOP STRUCTURE ======
    // Outer loop with multiple basic blocks
    for (int i = 0; i < SIZE; i += 2) {
        // First basic block in outer loop
        accumulator += array[i][0];
        
        // Conditional that creates partial overlap
        if (g_condition || (i % 3 == 0)) {
            // Middle loop - not fully contained in outer loop
            // because it has blocks outside the if condition
            int j = i;
            while (j < SIZE && j < i + CHUNK) {
                // Multiple basic blocks in middle loop
                if (j % 2 == 0) {
                    accumulator += array[i][j];
                    j++;
                    continue;  // Creates another basic block
                }
                
                // Inner loop - partially overlaps with middle loop
                // due to the break statement creating separate blocks
                for (int k = 0; k < CHUNK; k++) {
                    // Complex body with multiple blocks
                    if (k > j) {
                        accumulator -= array[j][k];
                        break;  // Creates exit block
                    } else {
                        accumulator += array[k][j];
                        if (accumulator > 1000) {
                            // Another basic block
                            accumulator = accumulator % 1000;
                        }
                    }
                    
                    // Label and goto to create additional blocks
                    if (k == CHUNK/2) {
                        goto mid_loop_label;
                    }
                    
                    // Dummy computation
                    g_volatile_counter++;
                }
                
            mid_loop_label:
                // This label creates another basic block
                j += (accumulator % 2) + 1;
                
                // Do-while to mix loop types
                int m = 0;
                do {
                    accumulator += m;
                    m++;
                    if (m > 5) break;
                } while (m < 3);  // Condition ensures it runs
            }
        } else {
            // Alternative path in outer loop
            // This creates blocks in outer loop that aren't in middle loop
            for (int alt = 0; alt < 10; alt++) {
                accumulator -= alt;
            }
        }
        
        // Final basic block in outer loop
        if (accumulator < 0) {
            accumulator = -accumulator;
        }
    }
    
    // ====== SECOND NESTING PATTERN ======
    // Different nesting to trigger more bitmap comparisons
    volatile int toggle = 0;
    
    for (int x = 0; x < SIZE/2; x++) {
        // Conditional that sometimes skips inner loops
        if (toggle || (x % 4 == 0)) {
            int y = x;
            do {
                // Inner loop with volatile condition
                for (int z = 0; z < SIZE/4; z++) {
                    if (g_volatile_counter++ % 7 == 0) {
                        // Nested if creates more blocks
                        accumulator += array[x][y] * array[y][z];
                        if (z > y) {
                            break;
                        }
                    } else {
                        accumulator -= z;
                        continue;
                    }
                    
                    // Another level of nesting
                    int w = 0;
                    while (w < 5) {
                        accumulator += w;
                        w += (toggle % 2);
                        toggle = !toggle;
                    }
                }
                y += 2;
            } while (y < SIZE && y < x * 3);
        }
        
        // More outer loop blocks
        toggle = (toggle + 1) % 3;
    }
    
    // ====== THIRD PATTERN WITH GOTO ======
    // Using goto to create non-standard control flow
    int counter = 0;
    int limit = 50;
    
    loop_start:
    if (counter++ < limit) {
        // Loop with goto creates interesting bitmap
        for (int a = 0; a < 10; a++) {
            if (a % 3 == g_condition) {
                accumulator += a;
                if (accumulator > 500) {
                    goto loop_end;
                }
            }
        }
        
        // Middle loop that shares some blocks with outer
        int b = 0;
        while (b < 8) {
            accumulator -= b;
            b += (counter % 2);
            
            // Innermost with fixed count
            for (int c = 0; c < 6; c++) {
                if (c == b) continue;
                accumulator += c * b;
            }
        }
        
        goto loop_start;
    }
    loop_end:
    
    // Prevent optimization
    printf("Result: %d (volatile: %d)\n", accumulator, g_volatile_counter);
    
    // Use result to prevent dead code elimination
    if (accumulator > 0) {
        return 0;
    } else {
        return 1;
    }
}

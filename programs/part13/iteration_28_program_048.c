#include <stdio.h>
#include <stdlib.h>

#define SIZE 64

volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;

int main() {
    int array[SIZE][SIZE];
    int result = 0;
    
    // Initialize array with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * j + 1;
        }
    }
    
    // Outer loop with multiple basic blocks
    for (int i = 0; i < SIZE; i++) {
        // First basic block in outer loop
        g_volatile_counter++;
        
        // Conditional that creates partial overlap
        if (i % 3 != 0) {
            // Middle loop - not fully contained in outer loop
            // because of the if condition above
            int j = 0;
            while (j < SIZE) {
                // Multiple basic blocks in middle loop
                if (j % 2 == 0) {
                    // Inner loop with volatile condition
                    for (int k = 0; k < SIZE; k++) {
                        // Complex inner loop body
                        if (k % 4 == 0) {
                            result += array[i][j] * k;
                            // Continue creates another basic block
                            continue;
                        } else if (k % 5 == 0) {
                            result -= array[j][k] * i;
                            // Break creates control flow complexity
                            if (g_volatile_counter > 100) break;
                        }
                        
                        // Another basic block
                        result ^= (array[i][k] & 0xFF);
                        
                        // Label and goto for additional complexity
                        if (result < 0) {
                            result = -result;
                        }
                    }
                } else {
                    // Alternative path in middle loop
                    // This creates blocks in middle loop not in inner loop
                    result += array[j][i] * 2;
                    
                    // Do-while loop for mixed loop types
                    int m = 0;
                    do {
                        result ^= (i * j + m);
                        m++;
                        // Volatile condition prevents optimization
                    } while (m < (g_volatile_counter % 8));
                }
                
                // Increment with condition
                j += (i % 4) + 1;
                
                // Another basic block at end of middle loop
                if (j > SIZE / 2) {
                    result >>= 1;
                }
            }
        } else {
            // This else branch contains blocks in outer loop
            // but not in middle/inner loops
            for (int x = 0; x < 10; x++) {
                result += x * i;
            }
        }
        
        // Final basic block in outer loop
        if (result > 1000000) {
            result %= 1000000;
        }
    }
    
    // Additional nested loop structure with different pattern
    volatile int trigger = 1;
    int counter = 0;
    
    // Triple nested loops with partial overlaps
    for (int a = 0; a < 32; a++) {
        // Outer loop block A
        counter += a;
        
        if (trigger || (a % 7 == 0)) {
            // Middle loop B
            int b = a;
            while (b < 32) {
                // Middle loop block B1
                if (b % 3 == 0) {
                    // Inner loop C - partially overlaps with B
                    for (int c = 0; c < 16; c++) {
                        // Complex inner loop body
                        array[a][b] += c;
                        
                        if (c % 6 == 0) {
                            // Nested if creates more blocks
                            array[b][c] -= a;
                            continue;
                        }
                        
                        // Another block in inner loop
                        counter ^= (a * b + c);
                    }
                }
                
                // Block in B but not in C
                b += (trigger ? 2 : 1);
                
                if (b % 5 == 0) {
                    // Another inner loop D - overlaps differently
                    int d = 0;
                    do {
                        array[d][a] = b;
                        d++;
                    } while (d < 8 && trigger);
                }
            }
        }
        
        // Block in A but not in B/C/D
        if (a % 11 == 0) {
            counter <<= 1;
        }
    }
    
    // Third nesting pattern with goto for additional complexity
    int x = 0, y = 0, z = 0;
    
outer_loop:
    for (x = 0; x < 20; x++) {
        if (x % 3 == g_outer_skip) {
            y = 0;
middle_loop:
            while (y < 15) {
                // Multiple blocks in middle loop
                result += array[x][y];
                
                if (y % 4 == 0) {
                    z = 0;
inner_loop:
                    for (z = 0; z < 10; z++) {
                        // Complex inner loop with goto
                        result -= array[y][z];
                        
                        if (result < 0) {
                            result = -result;
                            // Jump to label creates interesting CFG
                            if (z % 2 == 0) goto loop_end;
                        }
                        
                        // Another block
                        counter++;
                    }
                }
                
                y++;
                
                if (y == 10 && trigger) {
                    // Skip to outer loop sometimes
                    x++;
                    goto outer_loop;
                }
            }
        }
        
loop_end:
        // Empty block for CFG complexity
        ;
    }
    
    printf("Result: %d, Counter: %d\n", result, counter);
    return result != 0 ? 0 : 1;
}

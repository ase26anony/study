#include <stdio.h>
#include <stdlib.h>

#define SIZE 100

volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;
volatile int g_inner_skip = 1;

int main() {
    int array[SIZE][SIZE];
    volatile int accumulator = 0;
    volatile int loop_control = 50;
    
    // Initialize array with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * j + 1;
        }
    }
    
    // Outer loop - Level 1
    for (int i = 0; i < loop_control; i += 2) {
        // Multiple basic blocks in outer loop body
        if (g_outer_skip) {
            // Skip path - creates separate basic block
            accumulator += i;
            continue;
        }
        
        // Middle loop - Level 2 (not strictly contained)
        // This loop starts in outer loop but may exit early
        int j = 0;
        while (j < loop_control) {
            // Multiple basic blocks in middle loop
            if (j % 3 == 0) {
                // Early continue creates separate block
                j++;
                continue;
            }
            
            // Conditional that sometimes skips inner loop
            if (i % 4 == 0) {
                // Skip inner loop entirely for some iterations
                accumulator += array[i][j];
                j += 2;
                continue;
            }
            
            // Inner loop - Level 3 (not strictly contained in middle)
            // Do-while with volatile condition
            int k = 0;
            do {
                // Complex body with multiple blocks
                if (g_inner_skip && k % 5 == 0) {
                    // Skip computation
                    k++;
                    if (k > 10) break; // Early break
                    continue;
                }
                
                // Actual computation
                accumulator += array[i][j] * k;
                
                // Label for potential goto (creates basic block)
                retry_point:
                if (accumulator % 7 == 0) {
                    accumulator -= 1;
                }
                
                k++;
                
                // Volatile check in condition
            } while (k < (j % 8 + 5) && g_volatile_counter == 0);
            
            // Middle loop increment with condition
            if (accumulator > 1000) {
                j += 3;
            } else {
                j++;
            }
            
            // Another basic block in middle loop
            g_volatile_counter = (g_volatile_counter + 1) % 10;
        }
        
        // Additional basic block in outer loop
        if (i % 10 == 0) {
            // Nested if-else creates more blocks
            if (accumulator < 0) {
                accumulator = 0;
            } else {
                accumulator /= 2;
            }
        }
    }
    
    // Second set of loops with different structure
    volatile int alt_control = 30;
    
    // Another outer loop
    for (int x = 10; x < alt_control; x++) {
        // Conditional with partial overlap
        if (x % 2 == 0) {
            // Middle loop that overlaps but isn't subset
            for (int y = x; y < alt_control + 5; y += 2) {
                // Inner loop with goto
                for (int z = 0; z < y % 10; z++) {
                    if (z == 3) goto skip_inner;
                    accumulator += x * y * z;
                    
                    skip_inner:
                    if (z == 5) continue;
                }
                
                // Break from middle loop sometimes
                if (y > 20) break;
            }
        } else {
            // Different path - middle loop not entered
            accumulator -= x;
        }
    }
    
    // Third nesting pattern with constant bounds
    const int N = 25;
    for (int a = 0; a < N; a++) {
        // Loop with switch inside
        for (int b = a; b < N; b++) {
            switch (b % 4) {
                case 0:
                    // Inner loop in case 0 only
                    for (int c = 0; c < b % 6; c++) {
                        accumulator += a + b + c;
                    }
                    break;
                case 1:
                    // Different inner loop
                    int d = 0;
                    while (d < 5) {
                        accumulator -= d;
                        d++;
                    }
                    break;
                default:
                    // No inner loop
                    accumulator *= 2;
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return accumulator != 0 ? 0 : 1;
}

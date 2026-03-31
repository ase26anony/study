#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 20

// Volatile variables to prevent optimization
volatile int v_counter = 0;
volatile int v_trigger = 1;
volatile int v_mode = 0;

// Complex array structure
int data[SIZE][SIZE];
int result[SIZE][SIZE];
int temp[SIZE];

int main() {
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            data[i][j] = (i * j) % 100;
        }
        temp[i] = i % 10;
    }
    
    // Outer loop with multiple basic blocks
    // This creates a complex block bitmap
    for (int i = 0; i < SIZE; i++) {
        // First basic block in outer loop
        v_counter++;
        
        // Conditional that creates branching
        if (v_mode == 0) {
            // Middle loop - NOT strictly contained in outer loop
            // because it has blocks outside (the increment/decrement)
            int j = 0;
            while (j < SIZE) {
                // Multiple basic blocks in middle loop
                if (data[i][j] > 50) {
                    // Inner loop - partially overlaps with middle loop
                    // Fixed iteration count to encourage hardware loops
                    for (int k = 0; k < CHUNK; k++) {
                        // Complex inner loop body
                        if (k % 2 == 0) {
                            result[i][j] += data[i][j] * k;
                            // Continue creates additional basic block
                            if (result[i][j] > 1000) continue;
                        } else {
                            result[i][j] -= data[i][j] / (k + 1);
                            // Break creates another basic block
                            if (result[i][j] < -100) break;
                        }
                        // Label and goto for additional complexity
                        update_point:
                        v_counter += (i + j + k) % 7;
                    }
                } else {
                    // Alternative path in middle loop
                    // This creates blocks in middle loop that aren't in inner loop
                    result[i][j] = data[i][j] * 2;
                    // Goto creates edge case
                    if (result[i][j] == 66) goto update_point;
                }
                
                // Increment with volatile check
                j += (v_trigger > 0) ? 1 : 2;
                
                // Additional basic block for overflow check
                if (j > SIZE * 2) {
                    j = SIZE; // Force exit
                }
            }
            
            // Block after middle loop but still in outer loop
            v_mode = (v_mode + 1) % 3;
        } else if (v_mode == 1) {
            // Different path in outer loop
            // Do-while loop for variety
            int m = 0;
            do {
                temp[m] += i * m;
                m++;
                // Volatile condition
            } while (m < SIZE && v_trigger > 0);
            
            v_mode = 2;
        } else {
            // Third path with nested loops but different structure
            for (int n = i; n < SIZE && n < i + 10; n++) {
                int p = 0;
                // While loop inside for loop
                while (p < SIZE) {
                    result[n][p] = data[n][p] + temp[p];
                    p += (n % 3) + 1;
                    
                    // Early exit based on volatile
                    if (v_trigger < 0) {
                        goto early_exit;
                    }
                }
                early_exit:
                v_counter -= n % 5;
            }
            v_mode = 0;
        }
        
        // Final basic block in outer loop
        if (i % 33 == 0) {
            v_trigger = (v_trigger * 7) % 13;
        }
    }
    
    // Second set of loops with different nesting pattern
    // Creates additional opportunities for bitmap intersection checks
    volatile int acc = 0;
    
    // Triple nested loops with partial overlaps
    for (int x = 0; x < SIZE / 2; x++) {
        // Conditional that sometimes skips inner loops
        if (x % 4 != 0) {
            int y = x;
            // Middle loop with variable bounds
            while (y < SIZE && y < x * 2) {
                // Inner loop that's not strictly contained
                for (int z = 0; z < SIZE; z++) {
                    if (z % 2 == 0) {
                        acc += data[x][z] - result[y][z];
                        // Nested if for more basic blocks
                        if (acc > 10000) {
                            acc = 10000;
                        }
                    } else {
                        acc -= temp[z] * 3;
                        if (acc < -5000) {
                            acc = -5000;
                            // Continue to different block
                            continue;
                        }
                    }
                    
                    // Additional computation
                    result[x][y] = acc % 1000;
                }
                
                y += (x % 3) + 1;
                
                // Break based on volatile
                if (v_trigger == 0) {
                    break;
                }
            }
        } else {
            // Alternative path that still has loops
            for (int y = 0; y < 10; y++) {
                acc += y * x;
            }
        }
        
        // Modify volatile to affect inner loops
        if (x % 7 == 0) {
            v_trigger = (v_trigger + 1) % 5;
        }
    }
    
    // Use results to prevent elimination
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            final_sum = (final_sum + result[i][j]) % 1000000;
        }
        final_sum = (final_sum + temp[i]) % 1000000;
    }
    
    final_sum = (final_sum + acc + v_counter) % 1000000;
    
    printf("Result: %d\n", final_sum);
    return final_sum;
}

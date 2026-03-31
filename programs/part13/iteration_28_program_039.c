#include <stdio.h>
#include <stdlib.h>

// Volatile variables to prevent optimization
volatile int g_outer_cond = 1;
volatile int g_middle_cond = 1;
volatile int g_inner_cond = 1;
volatile int g_counter = 0;
volatile int g_accumulator = 0;

// Function to create complex control flow within loops
int process_value(int x, int y, int z) {
    volatile int result = 0;
    
    // Multiple basic blocks within a function
    if (x % 2 == 0) {
        result = y * z;
        if (result > 100) {
            result /= 2;
            goto adjust;
        }
    } else {
        result = y + z;
        if (result < 50) {
            result *= 3;
        }
    }
    
adjust:
    // Another basic block
    result += (x % 3);
    
    // Label for goto to create additional blocks
    if (result % 5 == 0) {
        result -= 10;
    }
    
    return result;
}

int main() {
    const int SIZE = 100;
    int array1[SIZE][SIZE];
    int array2[SIZE][SIZE];
    volatile int sum = 0;
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array1[i][j] = i * j;
            array2[i][j] = i + j;
        }
    }
    
    // LEVEL 1: Outer loop with complex control flow
    // This creates multiple basic blocks in the outer loop
    for (int i = 0; i < SIZE; i += 2) {  // Fixed iteration count
        // First basic block in outer loop
        g_counter++;
        
        // Conditional that creates branch
        if (g_outer_cond && (i % 3 == 0)) {
            // This block is sometimes executed
            
            // LEVEL 2: Middle loop - NOT strictly contained in outer loop
            // because it's inside a conditional
            int j = 0;
            while (j < SIZE) {  // Mixed loop type
                // Multiple basic blocks in middle loop
                if (array1[i][j] > 500) {
                    // Early continue creates another block
                    j += 3;
                    continue;
                }
                
                // Another conditional inside middle loop
                if (g_middle_cond && (j % 4 == 0)) {
                    // LEVEL 3: Inner loop - NOT strictly contained in middle loop
                    // because it's inside another conditional
                    for (int k = 0; k < SIZE / 2; k += 1) {  // Fixed iteration count
                        // Complex body with multiple blocks
                        int val = process_value(i, j, k);
                        
                        // Conditional break
                        if (val > 1000 && g_inner_cond) {
                            // Break creates another block
                            sum += val;
                            break;
                        }
                        
                        // Continue with normal processing
                        sum += array2[i][k] - val;
                        
                        // Another conditional
                        if (k % 7 == 0) {
                            sum -= array1[j][k];
                            goto inner_label;
                        }
                        
                        sum += 1;
                        
                    inner_label:
                        // Label for goto
                        g_accumulator += (i + j + k) % 11;
                    }
                } else {
                    // Alternative path in middle loop
                    sum += array1[i][j] * 2;
                }
                
                // Update with volatile to prevent optimization
                j += 1 + (g_counter % 2);
            }
        } else {
            // Alternative path in outer loop
            // This block is NOT part of the middle/inner loops
            for (int x = 0; x < 10; x++) {
                sum -= x * i;
            }
        }
        
        // Final block in outer loop (executed after conditional)
        g_accumulator += i % 13;
    }
    
    // SECOND NESTING PATTERN with different overlap characteristics
    volatile int alt_sum = 0;
    
    // Another outer loop
    for (int a = SIZE-1; a >= 0; a -= 3) {
        // Multiple blocks in this loop too
        alt_sum += a;
        
        if (a % 5 != 0) {
            // Middle loop that starts here but may extend beyond
            int b = a;
            do {  // do-while loop for variety
                alt_sum -= b;
                
                if (b % 3 == 0 && g_inner_cond) {
                    // Inner loop with partial overlap
                    int c = 0;
                    while (c < b && g_middle_cond) {
                        alt_sum += array1[a][c] * array2[b][c];
                        
                        // Complex control within inner loop
                        if (alt_sum > 10000) {
                            alt_sum /= 2;
                            c += 5;  // Skip ahead
                            continue;
                        }
                        
                        c += 2;
                    }
                }
                
                b -= 2;
            } while (b > 0 && alt_sum < 5000);
        }
        
        // More code in outer loop after the middle loop
        alt_sum *= (a % 7 + 1);
    }
    
    // THIRD pattern: Interleaved loops with shared blocks
    volatile int shared = 0;
    int toggle = 0;
    
    for (int x = 0; x < 50; x++) {
        shared += x;
        
        // This condition creates partial overlap
        if (toggle) {
            for (int y = x; y < 50; y++) {
                shared += y;
                
                // Another condition inside
                if (y % 2 == g_outer_cond) {
                    int z = 0;
                    while (z < 20) {
                        shared -= z;
                        z += 1 + (y % 3);
                        
                        // Early exit
                        if (shared < -1000) goto reset_shared;
                    }
                }
            }
        }
        toggle = !toggle;
        
    reset_shared:
        if (shared < -1000) shared = 0;
    }
    
    // Combine results to prevent elimination
    int result = sum + alt_sum + shared + g_accumulator;
    
    printf("Result: %d\n", result);
    
    // Use result to prevent dead code elimination
    return result % 255;
}

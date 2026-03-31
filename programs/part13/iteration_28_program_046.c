#include <stdio.h>
#include <stdlib.h>

// Volatile variables to prevent optimization
volatile int g_volatile_counter = 0;
volatile int g_outer_cond = 1;
volatile int g_middle_cond = 1;
volatile int g_inner_cond = 1;

// Function to create complex control flow within loops
int complex_condition(int x, int y) {
    volatile int a = x * 3;
    volatile int b = y * 7;
    return (a % 5) > (b % 3);
}

int main() {
    // Multi-dimensional arrays for loop operations
    int array1[100][100];
    int array2[100][100];
    int result[100][100];
    
    // Initialize arrays
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            array1[i][j] = i + j;
            array2[i][j] = i * j;
        }
    }
    
    // Outer loop with multiple basic blocks
    // This creates a complex block bitmap
    for (int i = 0; i < 50; i++) {
        // First basic block in outer loop
        int outer_acc = 0;
        
        // Conditional that creates branching
        if (g_outer_cond && (i % 3 == 0)) {
            // This branch contains the middle loop
            // Middle loop - NOT strictly contained in outer loop
            int j = 0;
            while (j < 40) {
                // Multiple basic blocks in middle loop
                if (complex_condition(i, j)) {
                    // Inner loop - NOT strictly contained in middle loop
                    // This creates partial overlap
                    for (int k = 0; k < 30; k++) {
                        // Complex inner loop body
                        volatile int temp = array1[i][j] + array2[j][k];
                        
                        // Conditional break - creates additional basic blocks
                        if (temp > 1000) {
                            result[i][j] += temp;
                            break;  // Creates exit block
                        }
                        
                        // Continue possibility
                        if (temp < 0) {
                            result[i][j] -= temp;
                            continue;
                        }
                        
                        // Normal computation
                        result[i][j] = result[i][j] * 2 + temp;
                        
                        // Label and goto to create more complex CFG
                        if (k % 7 == 0) {
                            result[i][j] /= 2;
                        }
                    }
                    
                    // Additional computation after inner loop
                    // This block is in middle loop but NOT in inner loop
                    result[i][j] += i * j;
                } else {
                    // Alternative path in middle loop
                    // This creates blocks in middle loop that are NOT in inner loop
                    for (int k = 10; k < 20; k++) {
                        result[i][j] -= array2[j][k];
                    }
                }
                
                // Increment with condition
                j += (j % 2 == 0) ? 1 : 2;
                
                // Volatile check to prevent optimization
                if (g_middle_cond) {
                    g_volatile_counter++;
                }
            }
            
            // Block after middle loop in the if branch
            // This is in outer loop but NOT in middle loop
            outer_acc += i * 10;
        } else {
            // Alternative branch in outer loop
            // This creates blocks in outer loop that are NOT in middle loop
            for (int j = 20; j < 30; j++) {
                // Different inner loop structure
                int k = 0;
                do {
                    result[i][j] = array1[i][j] - array2[i][k];
                    k++;
                } while (k < 15 && g_inner_cond);
            }
        }
        
        // Final computation in outer loop
        // This block is in outer loop but may not be in other loops
        result[i][0] += outer_acc;
        
        // Complex exit condition with volatile
        if (g_volatile_counter > 1000) {
            break;
        }
    }
    
    // Second set of nested loops with different structure
    // Creates additional loop relationships for analysis
    for (int x = 10; x < 30; x++) {
        // Loop with goto to create irreducible control flow
        int y = 0;
    start_y_loop:
        if (y < 20) {
            // Nested loop with switch inside
            for (int z = 0; z < 15; z++) {
                switch (z % 4) {
                    case 0:
                        result[x][y] += x;
                        break;
                    case 1:
                        result[x][y] += y;
                        // Fall through
                    case 2:
                        result[x][y] += z;
                        break;
                    default:
                        result[x][y] -= 1;
                }
                
                // Conditional continue
                if (z == 10) {
                    y++;
                    goto start_y_loop;  // Creates non-standard loop structure
                }
            }
            y++;
            goto start_y_loop;
        }
    }
    
    // Third level: do-while loops mixed with for loops
    int a = 0;
    do {
        // Outer do-while
        int b = 5;
        while (b < 25) {
            // Middle while loop
            for (int c = b; c < b + 10; c++) {
                // Inner for loop
                if (a + b + c < 100) {
                    result[a % 50][b % 50] += c;
                } else {
                    // Break to middle loop
                    result[a % 50][b % 50] -= c;
                    break;
                }
                
                // Multiple basic blocks
                volatile int check = result[a % 50][b % 50];
                if (check < 0) {
                    result[a % 50][b % 50] = 0;
                }
            }
            
            // Update with volatile condition
            b += (g_volatile_counter % 3) + 1;
        }
        a++;
    } while (a < 20 && g_outer_cond);
    
    // Compute final result to prevent dead code elimination
    int final_sum = 0;
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            final_sum += result[i][j];
            // Volatile access
            g_volatile_counter += result[i][j] % 100;
        }
    }
    
    printf("Final result: %d (volatile counter: %d)\n", 
           final_sum % 1000, g_volatile_counter);
    
    return final_sum % 255;
}

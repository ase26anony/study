#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_SIZE 50

volatile int g_outer_cond = 1;
volatile int g_middle_cond = 1;
volatile int g_inner_cond = 1;
volatile int g_counter = 0;
volatile int g_prevent_opt = 0;

int main() {
    int array1[SIZE][SIZE];
    int array2[SIZE][SIZE];
    int result = 0;
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array1[i][j] = i + j;
            array2[i][j] = i * j;
        }
    }
    
    // Outer loop - Level 1
    for (int i = 0; i < SIZE; i++) {
        // Multiple basic blocks in outer loop body
        if (g_outer_cond) {
            // This block is part of outer loop but not in middle loop
            
            // Middle loop - Level 2 (not strictly contained in outer)
            // Starts in one branch, continues in another
            int j = 0;
            while (j < SIZE) {
                // Multiple basic blocks in middle loop
                if (j % 2 == 0) {
                    // Inner loop - Level 3 (not strictly contained in middle)
                    for (int k = 0; k < INNER_SIZE; k++) {
                        // Complex inner loop body with multiple blocks
                        if (g_inner_cond) {
                            result += array1[i][j] * array2[j][k];
                            g_counter++;
                            
                            // Early exit condition
                            if (result > 1000000) {
                                result = result % 1000;
                            }
                        } else {
                            result -= array1[j][k] * array2[k][i];
                            g_counter--;
                        }
                        
                        // Label and goto to create additional basic blocks
                        if (k == INNER_SIZE / 2) {
                            goto mid_point;
                        }
                        
                        // Continue with normal processing
                        g_prevent_opt += (i * j * k) & 0xFF;
                        continue;
                        
                    mid_point:
                        // This creates another basic block
                        g_prevent_opt += (i + j + k) & 0xFF;
                    }
                    
                    // Break sometimes to create exit block
                    if (g_counter > 1000) {
                        break;
                    }
                } else {
                    // Alternative path in middle loop
                    // This block is in middle loop but not in inner loop
                    result += array2[i][j];
                    g_prevent_opt += i & 0xF;
                }
                
                // Continue condition with volatile check
                if (g_middle_cond) {
                    j++;
                } else {
                    j += 2;
                }
            }
            
            // Post-middle loop processing in outer loop
            g_prevent_opt += result & 0xFF;
        } else {
            // Alternative outer loop path
            // This creates blocks in outer loop that are NOT in middle loop
            for (int x = 0; x < 10; x++) {
                result += x * i;
                g_prevent_opt += x;
            }
        }
        
        // Another conditional in outer loop
        switch (i % 3) {
            case 0:
                result += 1;
                break;
            case 1:
                result += 2;
                // Fall through creates another block
            case 2:
                result += 3;
                break;
        }
    }
    
    // Additional nested structure with do-while
    int a = 0, b = 0, c = 0;
    do {
        // Outer do-while loop
        b = 0;
        while (b < 20) {
            // Middle while loop
            for (c = 0; c < 15; c++) {
                // Inner for loop
                if ((a + b + c) % 2 == 0) {
                    result += a * b * c;
                } else {
                    result -= a + b + c;
                }
                
                // Create multiple exit points
                if (result < -1000) {
                    goto adjust_result;
                }
            }
            
            // This block is in while but not in for
            g_prevent_opt += b;
            b++;
        }
        
        // This block is in do-while but not in while
        result = result % 1000;
        a++;
        
    adjust_result:
        // Label creates another basic block
        if (result < 0) result = -result;
        
    } while (a < 10 && g_outer_cond);
    
    // Final computation to prevent elimination
    printf("Result: %d\n", result);
    printf("Counter: %d\n", g_counter);
    printf("PreventOpt: %d\n", g_prevent_opt);
    
    return result != 0 ? 0 : 1;
}

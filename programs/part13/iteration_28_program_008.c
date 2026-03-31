#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_BOUND 50
#define MIDDLE_BOUND 75

volatile int g_volatile_counter = 0;
volatile int g_condition_seed = 1;

int main() {
    // Multi-dimensional arrays to work with
    int array1[SIZE][SIZE];
    int array2[SIZE][SIZE];
    int result[SIZE][SIZE] = {0};
    
    // Initialize arrays with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array1[i][j] = i + j;
            array2[i][j] = i * j;
        }
    }
    
    volatile int outer_volatile = 10;
    volatile int middle_volatile = 20;
    volatile int inner_volatile = 30;
    
    int total_sum = 0;
    
    // OUTER LOOP - Level 1
    // This loop has multiple basic blocks due to internal conditions
    for (int i = 0; i < outer_volatile; i++) {
        // First basic block in outer loop
        g_volatile_counter++;
        
        // Conditional that creates branching in outer loop
        if (i % 3 == 0) {
            // This branch sometimes skips the middle loop entirely
            // creating partial overlap in block bitmaps
            result[i][0] += array1[i][0] * 2;
            continue;  // Skip to next outer iteration
        }
        
        // MIDDLE LOOP - Level 2
        // Not strictly contained within outer loop due to the continue above
        // Uses while loop for mixed loop types
        int j = 0;
        volatile int while_condition = middle_volatile - i;
        
        while (j < while_condition) {
            // Multiple basic blocks in middle loop
            if (j % 2 == 0) {
                // Branch that sometimes skips inner loop
                result[i][j] += array1[i][j] * 3;
                
                // Nested condition creating more basic blocks
                if (g_condition_seed > 0) {
                    g_condition_seed--;
                    j++;
                    continue;  // Skip rest of middle loop body
                }
            }
            
            // INNER LOOP - Level 3
            // Not strictly contained within middle loop due to continues/breaks
            // Uses do-while for another loop type
            int k = 0;
            volatile int inner_limit = inner_volatile - j;
            
            do {
                // Complex inner loop body with multiple basic blocks
                if (k % 4 == 0) {
                    result[i][j] += array2[k][i];
                    // Early exit from inner loop sometimes
                    if (k > inner_limit / 2) {
                        break;
                    }
                } else if (k % 3 == 0) {
                    // Another branch
                    total_sum += array1[k][j];
                    
                    // Goto to create additional control flow complexity
                    if (total_sum > 1000) {
                        goto inner_loop_label;
                    }
                } else {
                    // Default case
                    total_sum -= array2[i][k];
                }
                
                inner_loop_label:
                k++;
                
                // Volatile check prevents optimization
                if (g_volatile_counter > 100) {
                    inner_volatile = inner_volatile / 2;
                }
                
            } while (k < inner_limit && k < INNER_BOUND);
            
            // Back to middle loop body
            j++;
            
            // Another condition in middle loop
            if (j > MIDDLE_BOUND) {
                // Break to outer loop, creating cross-level control flow
                goto middle_loop_exit;
            }
        }
        
        middle_loop_exit:
        // More code in outer loop after middle loop
        // This ensures outer loop has blocks not in middle loop
        for (int x = 0; x < 5; x++) {
            total_sum += x * i;
        }
        
        // Another conditional branch in outer loop
        if (i % 5 == 0) {
            // This block is in outer but not in any inner loop
            volatile int temp = i * 100;
            total_sum += temp;
        }
    }
    
    // Additional loop structure to create more complex bitmap relationships
    // This loop partially overlaps with the previous structure
    volatile int alt_volatile = 15;
    
    for (int a = 5; a < alt_volatile; a++) {
        // Shares some blocks with previous loops but not all
        if (a < 10) {
            // This enters a loop that overlaps with previous inner loops
            for (int b = a; b < alt_volatile; b++) {
                total_sum += array1[a][b];
                
                // Conditional that sometimes executes a nested loop
                if (b % 2 == 0) {
                    // Very short inner loop
                    for (int c = 0; c < 3; c++) {
                        total_sum += c * b;
                    }
                }
            }
        } else {
            // Different path that doesn't contain nested loops
            total_sum -= array2[a][0];
        }
    }
    
    // Print result to prevent elimination
    printf("Result: %d\n", total_sum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return total_sum % 256;
}

#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_SIZE 50

volatile int g_volatile_counter = 0;
volatile int g_condition = 1;

int main() {
    // Multi-dimensional arrays to work with
    int matrix[SIZE][SIZE];
    int result[SIZE] = {0};
    
    // Initialize with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
    }
    
    // Outer loop - level 1
    for (int outer = 0; outer < SIZE; outer++) {
        // Complex loop body with multiple basic blocks
        if (g_condition) {
            // First basic block in outer loop
            result[outer] += matrix[outer][0];
            
            // Middle loop - level 2 (not strictly contained in outer)
            // This loop starts in outer's body but has blocks outside
            int middle = 0;
            while (middle < SIZE) {
                // Multiple basic blocks in middle loop
                if (middle % 3 == 0) {
                    // Skip some iterations
                    middle += 2;
                    continue;  // Creates additional basic blocks
                }
                
                // Inner loop - level 3 (not strictly contained in middle)
                // Fixed iteration count to encourage hardware loop
                for (int inner = 0; inner < INNER_SIZE; inner++) {
                    // Complex inner loop body
                    if (inner % 2 == 0) {
                        // Branch creates separate basic block
                        matrix[outer][middle] += inner;
                        g_volatile_counter++;
                    } else {
                        // Another basic block
                        matrix[outer][middle] -= inner;
                        
                        // Early exit sometimes
                        if (g_volatile_counter > 1000) {
                            break;  // Creates exit block
                        }
                    }
                    
                    // Label and goto to create more complex CFG
                    if (inner == INNER_SIZE / 2) {
                        goto midpoint;
                    }
                    
                    // Normal path continues
                    result[outer] += matrix[outer][middle];
                    continue;
                    
                midpoint:
                    // Alternate path
                    result[outer] -= matrix[outer][middle];
                }
                
                // Post-inner loop code in middle loop
                middle++;
                
                // Conditional break in middle loop
                if (middle > outer + 10) {
                    break;
                }
            }
            
            // Another block in outer's if branch
            result[outer] *= 2;
        } else {
            // Alternate path in outer loop (creates blocks outside middle's bitmap)
            for (int k = 0; k < 5; k++) {
                result[outer] -= k;
            }
        }
        
        // Final block in outer loop
        g_volatile_counter--;
    }
    
    // Additional loop structure to create more complex nesting
    // Do-while loop with volatile condition
    int counter = 0;
    do {
        // Nested for inside do-while
        for (int i = 0; i < 20; i++) {
            // While loop inside for inside do-while
            int j = 0;
            while (j < i) {
                // Creates partial overlap scenario
                if (j % 2 == 0) {
                    result[j % SIZE] += g_volatile_counter;
                }
                j++;
                
                // Early exit creates separate exit block
                if (g_volatile_counter < -100) {
                    goto cleanup;
                }
            }
            
            // This block is in the for loop but not in the while loop
            g_volatile_counter += i;
        }
        counter++;
    } while (counter < 10 && g_volatile_counter < 1000);
    
cleanup:
    // Final computation to prevent elimination
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += result[i];
    }
    
    printf("Result: %d (volatile counter: %d)\n", final_sum, g_volatile_counter);
    return final_sum != 0 ? 0 : 1;
}

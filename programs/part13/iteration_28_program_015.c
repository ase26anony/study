#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_SIZE 50

volatile int g_outer_cond = 1;
volatile int g_middle_cond = 1;
volatile int g_inner_cond = 1;
volatile int g_counter = 0;

// Function to create complex control flow within loops
int complex_condition(int i, int j, int k) {
    volatile int x = i * j + k;
    return (x % 7) > 3;
}

int main() {
    int array[SIZE][SIZE];
    int result = 0;
    volatile int acc = 0;
    
    // Initialize array
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * SIZE + j;
        }
    }
    
    // Level 1: Outer loop with complex entry conditions
    for (int i = 0; i < SIZE; i++) {
        // Create multiple basic blocks in outer loop body
        if (g_outer_cond && (i % 3 != 0)) {
            // This block is sometimes executed, sometimes skipped
            acc += i;
            
            // Level 2: Middle loop - NOT strictly contained in outer loop
            // because it has blocks outside the outer loop's main path
            int j = 0;
            while (j < INNER_SIZE) {
                // Multiple basic blocks in middle loop
                if (complex_condition(i, j, 0)) {
                    // Branch creates separate basic block
                    array[i][j] *= 2;
                    j += 2;  // Skip increment creates different flow
                    continue;  // Creates additional basic block
                } else {
                    array[i][j] /= 2;
                }
                
                // Level 3: Inner loop - NOT strictly contained in middle loop
                // because it's conditional and has overlapping but not subset blocks
                if (g_middle_cond && (j % 4 == 0)) {
                    // Inner for loop with fixed bounds but volatile condition
                    for (int k = 0; k < INNER_SIZE; k++) {
                        // Complex inner loop body with multiple blocks
                        volatile int temp = k;
                        
                        if (g_inner_cond && (temp % 5 == 0)) {
                            // Nested if creates another basic block
                            array[i][j] += array[j][k];
                            if (k > INNER_SIZE/2) {
                                // Early exit creates different block
                                break;
                            }
                        } else {
                            array[i][j] -= array[k][i];
                        }
                        
                        // Label and goto to create additional control flow
                        if (k == INNER_SIZE/3) {
                            goto special_case;
                        }
                        
                        acc += array[i][j];
                        continue;
                        
                    special_case:
                        acc -= array[i][j];
                        g_counter++;
                    }
                } else {
                    // Alternative path in middle loop
                    array[i][j] = -array[i][j];
                }
                
                j++;
                
                // Another conditional to create more basic blocks
                if (j % 7 == 0) {
                    result += array[i][j % SIZE];
                }
            }
            
            // Post-middle loop code in outer loop
            if (i % 5 == 0) {
                acc *= 2;
            }
        } else {
            // Alternative outer loop path
            for (int x = 0; x < 10; x++) {
                acc -= x;
            }
        }
        
        // Outer loop continuation with another condition
        volatile int check = i;
        do {
            result += check % 11;
            check /= 2;
        } while (check > 0);
    }
    
    // Additional loop structure to create more complex nesting
    // This creates loops that partially overlap with previous ones
    for (int a = 10; a < SIZE - 10; a += 2) {
        // Loop with mixed increment
        int b = a;
        while (b < SIZE) {
            if (b % 3 == 0) {
                // Nested loop that starts here but extends beyond
                for (int c = b; c < b + 20 && c < SIZE; c++) {
                    array[a][b] += array[b][c];
                    acc++;
                    
                    // Conditional continue creates separate block
                    if (c % 6 == 0) continue;
                    
                    array[b][c] -= array[c][a];
                }
                b += 4;
            } else {
                b += 1;
            }
            
            // This code is in the while loop but not in the inner for
            result ^= array[a][b % SIZE];
        }
    }
    
    printf("Result: %d, Accumulator: %d, Counter: %d\n", 
           result, acc, g_counter);
    
    // Use results to prevent optimization
    return (result + acc + g_counter) % 256;
}

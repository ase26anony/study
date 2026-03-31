#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_SIZE 50

volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;
volatile int g_inner_skip = 1;

// Complex loop structure with partial overlaps
void complex_nested_loops(int arr[SIZE][SIZE]) {
    volatile int accumulator = 0;
    volatile int control_var = 0;
    
    // Outer loop - level 1
    for (int i = 0; i < SIZE; i++) {
        // Multiple basic blocks in outer loop body
        if (g_outer_skip && (i % 3 == 0)) {
            // Skip middle loop for some iterations
            accumulator += arr[i][0];
            continue;
        }
        
        // Middle loop - level 2 (not strictly contained in outer)
        // This loop starts in outer but may exit early
        int j = 0;
        while (j < SIZE) {
            // Multiple basic blocks in middle loop
            if (j > INNER_SIZE && control_var > 10) {
                // Early exit from middle loop
                break;
            }
            
            // Conditional block that sometimes executes inner loop
            if (!(j % 4 == 0 && g_inner_skip)) {
                // Inner loop - level 3 (not strictly contained in middle)
                for (int k = 0; k < INNER_SIZE; k++) {
                    // Complex inner loop body with multiple blocks
                    if (k % 2 == 0) {
                        accumulator += arr[i][j] * k;
                        if (accumulator > 1000) {
                            // Nested if creates another basic block
                            accumulator -= 50;
                        }
                    } else {
                        accumulator -= arr[j][k] / 2;
                        // Label and goto to create additional blocks
                        if (accumulator < -500) {
                            goto adjust_accumulator;
                        }
                    }
                    
                    // Dummy computation
                    control_var = (control_var + 1) % 20;
                }
                
                // Label for goto
                adjust_accumulator:
                if (accumulator < -400) {
                    accumulator += 200;
                }
            } else {
                // Alternative path without inner loop
                accumulator += arr[i][j] * 2;
                // Continue statement creates another block
                if (accumulator > 800) {
                    j += 2;
                    continue;
                }
            }
            
            // Post-inner loop computation
            j += (control_var % 3) + 1;
            
            // Another basic block
            if (j % 5 == 0) {
                accumulator /= 2;
            }
        }
        
        // Outer loop continuation block
        g_volatile_counter++;
        if (i % 7 == 0) {
            // Nested if-else in outer loop
            if (accumulator > 0) {
                accumulator = accumulator % 100;
            } else {
                accumulator = -accumulator % 100;
            }
        }
    }
    
    // Prevent dead code elimination
    printf("Accumulator: %d\n", accumulator);
}

// Additional complex loop structure with do-while
void mixed_loop_types(int arr[SIZE][SIZE]) {
    volatile int sum = 0;
    volatile int iter_control = SIZE / 2;
    
    // Do-while outer loop
    int outer_idx = 0;
    do {
        // For loop as middle loop
        for (int mid = outer_idx; mid < SIZE - outer_idx; mid++) {
            // Partial overlap: this loop doesn't always execute
            if (mid % (outer_idx + 2) != 0) {
                // While loop as inner loop
                int inner = 0;
                while (inner < iter_control) {
                    // Multiple blocks in while body
                    sum += arr[outer_idx][mid] * inner;
                    
                    if (sum > 10000) {
                        sum = sum % 1000;
                        // Break from while
                        break;
                    } else if (sum < -10000) {
                        sum = -sum % 1000;
                        // Continue
                        inner += 3;
                        continue;
                    }
                    
                    inner += (mid % 3) + 1;
                }
            }
            
            // Middle loop alternate path
            sum -= arr[mid][outer_idx];
            
            // Early exit condition for middle loop
            if (sum < -5000 && mid > SIZE / 2) {
                goto finish_middle;
            }
        }
        
        finish_middle:
        // Do-while body continues
        outer_idx += (iter_control % 4) + 1;
        iter_control = (iter_control + 1) % SIZE;
        
    } while (outer_idx < SIZE && g_volatile_counter < 100);
    
    printf("Mixed loops sum: %d\n", sum);
}

int main() {
    // Initialize array with non-zero values
    int (*array)[SIZE] = malloc(sizeof(int) * SIZE * SIZE);
    if (!array) return 1;
    
    // Fill with pattern
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = (i * 3 + j * 7) % 100;
        }
    }
    
    // Modify volatile variables to affect control flow
    g_outer_skip = 1;  // Will cause some outer iterations to skip middle loop
    g_inner_skip = 0;  // Enable inner loop execution
    
    // Execute complex nested loops
    complex_nested_loops(array);
    
    // Change volatile state
    g_volatile_counter = 50;
    g_outer_skip = 0;
    
    // Execute mixed loop types
    mixed_loop_types(array);
    
    // Final computation to prevent optimization
    volatile int final_result = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            // Nested loop with break
            for (int k = 0; k < 10; k++) {
                final_result += array[i][j] * k;
                if (final_result > 1000) {
                    break;  // Creates additional basic block
                }
            }
            
            // Continue with label
            if (j % 2 == 0) {
                final_result /= 2;
                continue;
            }
        }
    }
    
    printf("Final result: %d\n", final_result);
    
    free(array);
    return final_result % 256;
}

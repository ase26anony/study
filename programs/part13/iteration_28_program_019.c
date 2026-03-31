#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 20

// Volatile variables to prevent optimization
volatile int v_counter = 0;
volatile int v_trigger = 1;
volatile int v_mode = 0;

// Complex loop structure with partial overlaps
void process_matrix(int matrix[SIZE][SIZE]) {
    volatile int acc = 0;
    
    // Outer loop - level 1
    for (int i = 0; i < SIZE; i += 2) {
        // Multiple basic blocks in outer loop body
        if (v_mode == 0) {
            // Branch 1: Sometimes skip middle loop entirely
            if (i % 3 == 0) {
                acc += matrix[i][0];
                continue;  // Creates additional basic block
            }
        } else {
            // Branch 2: Different path
            acc -= matrix[i][0];
            if (v_trigger) {
                goto skip_middle;  // Creates label and goto
            }
        }
        
        // Middle loop - level 2 (not fully contained in outer)
        // This loop starts in outer but may exit to code outside outer's body
        int j = 0;
        while (j < SIZE) {
            // Complex middle loop body with multiple blocks
            if (j % 4 == 0) {
                // Nested if-else creates more blocks
                if (v_counter > 10) {
                    acc += matrix[i][j] * 2;
                } else {
                    acc += matrix[i][j];
                    break;  // Early exit creates separate block
                }
            } else {
                // Inner loop - level 3 (partially overlaps middle)
                // This is a do-while for variety
                int k = j;
                do {
                    // Inner loop body with condition
                    if (k % 5 == 0 && v_trigger) {
                        acc += matrix[i][k] / 2;
                        // Continue creates another block
                        continue;
                    }
                    acc += matrix[i][k];
                    
                    // Additional block with label
                    if (acc > 1000) {
                        goto inner_done;
                    }
                    
                    k++;
                } while (k < j + CHUNK && k < SIZE);
                
                inner_done:
                // Code after inner loop but still in middle
                if (v_counter++ > 50) {
                    v_counter = 0;
                }
            }
            
            j += (i % 2) + 1;  // Variable increment
        }
        
        skip_middle:
        // Code that's in outer loop but not in middle loop
        // This creates the partial overlap
        if (i % 7 == 0) {
            // Another conditional block
            for (int x = 0; x < 5; x++) {
                acc += x;  // Mini-loop not related to main nesting
            }
        }
    }
    
    // Additional outer loop that overlaps with first outer's blocks
    // This creates more complex bitmap relationships
    v_mode = 1;
    for (int i = SIZE-1; i >= 0; i--) {
        // This loop shares some blocks with the first outer loop
        // but has different structure
        int temp = 0;
        
        // While loop inside for - mixed types
        while (temp < 10) {
            // Conditional that sometimes executes, sometimes not
            if (v_trigger && i % 2 == 0) {
                // Another inner loop
                for (int z = 0; z < 3; z++) {
                    acc += matrix[i][temp] * z;
                    if (acc < 0) {
                        // Break to outer scope
                        goto outer_break;
                    }
                }
            }
            temp++;
        }
        
        if (i == SIZE/2) {
            v_trigger = !v_trigger;
        }
    }
    outer_break:
    
    printf("Accumulator: %d\n", acc);
}

// Helper function with different loop structure
void alternate_path(int arr[SIZE]) {
    volatile int sum = 0;
    
    // Triple nested loops with partial overlaps
    for (int a = 0; a < SIZE; a += 3) {
        // First level
        if (a % 2 == 0) {
            for (int b = a; b < a + CHUNK && b < SIZE; b++) {
                // Second level - partially overlaps first
                sum += arr[b];
                
                // Third level - partially overlaps second
                int c = b;
                while (c < b + 5 && c < SIZE) {
                    if (c % 3 == 0) {
                        sum -= arr[c];
                        c += 2;
                        continue;
                    }
                    sum += arr[c] * 2;
                    c++;
                }
                
                // Code in second but not in third
                if (sum > 10000) {
                    break;
                }
            }
        } else {
            // Alternate path in first level
            sum += arr[a] * 3;
        }
        
        // Code in first but not always in second
        if (a % 5 == 0) {
            goto skip_rest;
        }
    }
    skip_rest:
    
    v_counter = sum % 100;
}

int main() {
    // Initialize data
    int matrix[SIZE][SIZE];
    int array[SIZE];
    
    // Fill with pseudo-random data
    for (int i = 0; i < SIZE; i++) {
        array[i] = i * 3;
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * j) % 100;
        }
    }
    
    // Execute complex loop structures
    process_matrix(matrix);
    alternate_path(array);
    
    // Use results to prevent elimination
    volatile int final_result = v_counter + (int)matrix[0][0];
    printf("Final: %d\n", final_result);
    
    return final_result % 2;
}

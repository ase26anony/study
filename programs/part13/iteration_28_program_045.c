#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 20

// Volatile variables to prevent optimization
volatile int v_counter = 0;
volatile int v_trigger = 1;
volatile int v_switch = 0;

// Complex array structure
int matrix[SIZE][SIZE];
int partial[SIZE][CHUNK];
int result[SIZE] = {0};

// Function with goto to create additional basic blocks
void process_block(int *block, int size, volatile int *acc) {
    if (size <= 0) return;
    
    int i = 0;
    // Loop with multiple basic blocks due to if-else
    while (i < size) {
        if (i % 3 == 0) {
            *acc += block[i] * 2;
            i += 2;  // Skip next
        } else if (i % 3 == 1) {
            *acc += block[i] / 2;
            // Nested if inside
            if (block[i] > 50) {
                *acc -= 5;
            }
            i++;
        } else {
            *acc += block[i];
            // Early continue creates new block
            if (block[i] < 0) {
                i++;
                continue;
            }
            *acc += 1;
            i++;
        }
        
        // Another condition
        if (*acc > 1000) {
            *acc = *acc % 1000;
        }
    }
}

int main() {
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * j + i + j) % 100;
        }
        for (int j = 0; j < CHUNK; j++) {
            partial[i][j] = (i * 7 + j * 3) % 50;
        }
    }
    
    volatile int accumulator = 0;
    volatile int outer_mod = 0;
    
    // LEVEL 1: Outer loop with multiple basic blocks
    for (int i = 0; i < SIZE; i += 2) {  // Step 2 creates partial overlap
        // First basic block in outer loop
        result[i] = matrix[i][0];
        accumulator += i;
        
        // Conditional that sometimes skips inner loops
        if (i % 3 != 0) {  // 2/3 of iterations enter
            // LEVEL 2: Middle loop - NOT fully contained in outer
            // because it has blocks outside the if condition
            int j = 0;
            while (j < CHUNK) {  // while loop for variety
                // Multiple blocks in middle loop body
                if (j % 4 == 0) {
                    accumulator += partial[i][j] * 3;
                    j += 2;  // Skip
                    continue;  // Creates new block
                }
                
                // LEVEL 3: Inner loop - NOT fully contained in middle
                // because of the conditional around it
                if (v_trigger || (j % 5 == 0)) {
                    // for loop with constant bounds
                    for (int k = 0; k < 8; k += 1 + (k % 2)) {  // Variable step
                        // Complex inner body
                        int val = matrix[i][(j + k) % SIZE];
                        accumulator += val;
                        
                        // Nested if with else
                        if (val > 40) {
                            accumulator -= 2;
                            // break can create exit block
                            if (accumulator > 5000) break;
                        } else {
                            accumulator += 1;
                        }
                        
                        // Label and goto to create more blocks
                        if (k == 3) {
                            accumulator *= 2;
                        }
                    } // end for
                } // end if
                
                // Another block in middle loop
                accumulator += j;
                j++;
                
                // do-while for another loop type
                int m = 0;
                do {
                    accumulator -= m;
                    m++;
                    if (m > 3) break;
                } while (m < 5);  // Condition not known at compile time
            } // end while
            
            // Block after middle loop but still in if
            accumulator /= 2;
        } // end if
        
        // This block is in outer loop but NOT in the if
        // So middle loop doesn't contain all outer blocks
        // and outer doesn't contain all middle blocks
        result[i] = accumulator % 100;
        
        // Another conditional with different structure
        if (i % 7 == 0) {
            // Different loop structure
            for (int n = i; n < i + 5 && n < SIZE; n++) {
                accumulator += matrix[n][i % SIZE];
                // Early exit
                if (accumulator < -1000) goto cleanup;
            }
        }
        
        // Label for goto
        cleanup:
        v_counter++;
    }
    
    // Second set of loops with different nesting pattern
    volatile int alt_acc = 0;
    
    // Outer do-while
    int p = 0;
    do {
        // Conditional with switch
        switch (p % 4) {
            case 0:
                // Middle for loop
                for (int q = p; q < p + 10; q++) {
                    // Inner while with complex condition
                    int r = 0;
                    while (r < 5 && alt_acc < 10000) {
                        alt_acc += matrix[p % SIZE][r] * q;
                        r += 1 + (alt_acc % 2);  // Variable increment
                        
                        // Nested if-else chain
                        if (r == 2) {
                            alt_acc += 100;
                        } else if (r == 3) {
                            alt_acc -= 50;
                            continue;  // Skip to next iteration
                        }
                        
                        // Another block
                        alt_acc %= 1000;
                    }
                    
                    // Block after inner but still in middle
                    if (q % 3 == 0) break;  // Break from middle sometimes
                }
                break;
                
            case 1:
                // Different structure
                alt_acc += process_block(partial[p % SIZE], CHUNK, &alt_acc);
                break;
                
            default:
                // Simple increment
                alt_acc += p;
        }
        
        p++;
        if (p > 30) break;  // Conditional break
    } while (p < 50);
    
    // Final computation to prevent elimination
    int final_result = (accumulator + alt_acc) % 10000;
    
    // Use result to prevent dead code elimination
    for (int i = 0; i < SIZE; i++) {
        final_result += result[i];
    }
    
    printf("Result: %d\n", final_result);
    return final_result > 0 ? 0 : 1;
}

#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_BOUND 50

// Volatile variables to prevent optimization
volatile int v1 = 1;
volatile int v2 = 2;
volatile int v3 = 3;
volatile int v4 = 4;
volatile int v5 = 5;

// Global accumulator to prevent dead code elimination
volatile long long accumulator = 0;

int main() {
    // Multi-dimensional arrays to work with
    int matrix[SIZE][SIZE];
    int vector[SIZE];
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        vector[i] = i;
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    // Complex nested loop structure with partial overlaps
    
    // LEVEL 1: Outer loop with complex body
    for (int i = 0; i < SIZE; i += v1) {
        // First basic block in outer loop
        int temp = vector[i];
        
        // Conditional that creates partial overlap
        if (i % 3 == 0) {
            // This branch contains middle loop
            // LEVEL 2: Middle loop - not fully contained in outer loop
            int j = 0;
            while (j < SIZE) {
                // Multiple basic blocks in middle loop body
                if (j % 2 == 0) {
                    // LEVEL 3: Inner loop - partially overlaps with middle
                    for (int k = 0; k < INNER_BOUND; k += v2) {
                        // Complex inner loop body
                        if (k % 4 == 0) {
                            accumulator += matrix[i][j] * k;
                            // Early continue creates additional basic block
                            if (accumulator > 1000000) continue;
                        } else {
                            accumulator -= matrix[j][i] / (k + 1);
                        }
                        
                        // Label and goto to create more complex CFG
                        if (k == INNER_BOUND / 2) {
                            accumulator *= 2;
                        }
                    }
                    // End of inner loop
                } else {
                    // Alternative path in middle loop
                    accumulator += vector[j];
                    // Break can create exit block
                    if (accumulator < -1000000) break;
                }
                
                // Update with volatile to prevent simplification
                j += v3;
                
                // Continue creates another basic block
                if (j % 5 == 0) continue;
                
                // Dummy computation
                accumulator ^= j;
            }
            // End of middle loop
        } else if (i % 3 == 1) {
            // Different branch - contains another loop that's not nested
            // This creates blocks in outer loop that aren't in middle loop
            do {
                accumulator += i * v4;
                v4++;
            } while (v4 < 10);
        } else {
            // Third branch - simple computation
            accumulator -= i;
        }
        
        // Final part of outer loop body (not in middle loop)
        if (i % 7 == 0) {
            // Another conditional inside outer loop
            for (int x = 0; x < 5; x++) {
                accumulator += x * v5;
            }
        }
    }
    
    // Second set of loops with different nesting pattern
    // Creates more complex bitmap relationships
    
    int a = 0, b = 0, c = 0;
    
    // Loop with goto to create irreducible control flow
    for (a = 0; a < 20; a += v1) {
        if (a % 2 == 0) {
            b = 0;
            while (b < 15) {
                // Nested loop with early exit
                for (c = 0; c < 10; c++) {
                    accumulator += a * b * c;
                    if (accumulator > 5000000) goto early_exit;
                }
                b += v2;
                
                // This creates partial overlap - some blocks of while
                // are not in the for loop, and vice versa
                if (b % 3 == 0) {
                    accumulator >>= 1;
                }
            }
        }
    early_exit:
        // Label creates additional basic block
        if (a == 10) {
            accumulator |= 0xFF;
        }
    }
    
    // Third pattern: deeply nested with mixed types
    int counter = 0;
    
    do {
        // Outer do-while
        for (int layer1 = 0; layer1 < 8; layer1++) {
            if (layer1 % 2 == counter % 2) {
                int layer2 = layer1;
                while (layer2 < 10) {
                    // Innermost with switch for multiple blocks
                    switch (layer2 % 3) {
                        case 0:
                            accumulator += matrix[layer1][layer2];
                            break;
                        case 1:
                            accumulator -= vector[layer2];
                            // Fall through
                        case 2:
                            accumulator *= (layer1 + 1);
                            break;
                    }
                    layer2 += v3;
                }
            } else {
                // Alternative path that skips the while loop
                accumulator ^= layer1;
            }
        }
        counter++;
    } while (counter < 5 && accumulator < 10000000);
    
    // Print result to prevent elimination
    printf("Result: %lld\n", accumulator);
    
    // Use result to affect return value
    return (accumulator > 0) ? 0 : 1;
}

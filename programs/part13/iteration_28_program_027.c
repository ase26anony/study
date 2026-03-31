#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 25

volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;
volatile int g_inner_condition = 1;

int main() {
    int array[SIZE][SIZE];
    volatile int accumulator = 0;
    volatile int loop_control = SIZE / 2;
    
    // Initialize array with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * SIZE + j;
        }
    }
    
    // Outer loop with multiple basic blocks
    for (int i = 0; i < SIZE; i++) {
        // First basic block in outer loop
        accumulator += array[i][0];
        
        // Conditional that creates partial overlap
        if (i % 3 != 0) {
            // Middle loop - not always executed
            int j = 0;
            while (j < loop_control) {
                // Multiple basic blocks in middle loop
                if (j % 2 == 0) {
                    // Inner loop with volatile condition
                    for (int k = 0; k < CHUNK; k++) {
                        // Complex inner loop body
                        accumulator += array[i][j] * k;
                        
                        // Conditional break creates multiple blocks
                        if (g_inner_condition && k > CHUNK/2) {
                            accumulator -= array[j][k];
                            // Continue to next iteration
                            continue;
                        }
                        
                        // Another basic block
                        accumulator += array[k][i];
                        
                        // Label and goto to create additional blocks
                        if (accumulator > 1000) {
                            accumulator = accumulator / 2;
                        }
                    }
                } else {
                    // Alternative path in middle loop
                    do {
                        accumulator -= array[j][i];
                        j++;
                        // Volatile check prevents optimization
                        if (g_volatile_counter++ > 100) {
                            break;
                        }
                    } while (j < loop_control && accumulator < 5000);
                    
                    // Skip to next iteration
                    continue;
                }
                
                // Post-inner loop block in middle loop
                accumulator += j;
                j++;
                
                // Another conditional
                if (accumulator < 0) {
                    accumulator = -accumulator;
                }
            }
        } else {
            // Alternative outer loop path
            for (int alt = i; alt < i + 5 && alt < SIZE; alt++) {
                accumulator += array[alt][alt];
                
                // Nested do-while with different structure
                int counter = 0;
                do {
                    accumulator -= counter;
                    counter++;
                    if (counter > 10) break;
                } while (accumulator > -1000);
            }
        }
        
        // Final block in outer loop
        if (i == SIZE - 1) {
            accumulator *= 2;
        }
        
        // Volatile modification
        g_outer_skip = i % 4;
    }
    
    // Additional loop nest with different pattern
    volatile int dim = SIZE / 4;
    for (int x = 0; x < dim; x++) {
        // Conditional with goto
        if (x % 3 == 0) goto skip_middle;
        
        for (int y = x; y < dim + x && y < SIZE; y++) {
            int z = 0;
            while (z < dim) {
                accumulator += array[x][y] - array[y][z];
                z += 2;
                
                // Nested if-else chain
                if (z > dim/2) {
                    if (accumulator > 10000) {
                        accumulator = 0;
                    } else {
                        accumulator += 1;
                    }
                }
            }
        }
        
        skip_middle:
        // Loop that shares some blocks with previous but not all
        for (int y = 0; y < x; y++) {
            accumulator += array[y][x];
        }
    }
    
    // Print result to prevent elimination
    printf("Result: %d\n", accumulator);
    
    return accumulator != 0 ? 0 : 1;
}

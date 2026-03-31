#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_SIZE 50

volatile int g_volatile_counter = 0;
volatile int g_condition = 1;

int main() {
    int array[SIZE][SIZE];
    int result = 0;
    
    // Initialize array with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * SIZE + j;
        }
    }
    
    // Outer loop with multiple basic blocks
    for (int outer = 0; outer < SIZE; outer++) {
        // First basic block in outer loop
        volatile int outer_mod = outer % 3;
        
        // Conditional that creates partial overlap
        if (outer_mod == 0) {
            // Middle loop - not strictly contained in outer
            int middle = 0;
            while (middle < SIZE) {
                // Multiple basic blocks in middle loop
                if (middle % 2 == 0) {
                    // Inner loop with volatile condition
                    for (int inner = 0; inner < INNER_SIZE; inner++) {
                        // Complex body with multiple blocks
                        volatile int temp = array[outer][middle] + inner;
                        
                        if (temp % 5 == 0) {
                            result += temp;
                            g_volatile_counter++;
                        } else {
                            result -= temp;
                            // Continue creates another basic block
                            continue;
                        }
                        
                        // Another basic block
                        if (g_volatile_counter > 1000) {
                            break;  // Early exit creates another block
                        }
                    }
                } else {
                    // Alternative path in middle loop
                    result += array[outer][middle] * 2;
                    
                    // Another inner loop with different bounds
                    int k = 0;
                    do {
                        result -= k;
                        k++;
                        // Volatile check prevents optimization
                        if (g_condition) {
                            k += outer % 2;
                        }
                    } while (k < 10);
                }
                
                middle++;
                
                // Label and goto create additional basic blocks
                if (middle % 7 == 0) {
                    goto skip_increment;
                }
                
                middle += outer % 2;
                
            skip_increment:
                // Empty label block
                ;
            }
        } else if (outer_mod == 1) {
            // Different middle loop structure
            for (int j = 0; j < SIZE / 2; j++) {
                // Inner while loop
                int k = 0;
                while (k < j + 5) {
                    array[outer][j] += k;
                    k++;
                    
                    // Nested if-else creates more blocks
                    if (k % 3 == 0) {
                        result += array[outer][j];
                    } else {
                        result -= k;
                    }
                }
            }
        } else {
            // Third path with do-while loop
            int counter = 0;
            do {
                // Another inner for loop
                for (int k = 0; k < 8; k++) {
                    result += array[outer][counter] * k;
                    
                    // Volatile access
                    if (g_volatile_counter < 500) {
                        g_volatile_counter++;
                    }
                }
                counter++;
            } while (counter < 5);
        }
        
        // Final basic block in outer loop
        result += outer;
    }
    
    // Additional loop nest with different pattern
    volatile int seed = 42;
    for (int i = 0; i < 20; i++) {
        int j = 0;
        while (j < 15) {
            // Inner loop with break
            for (int k = 0; k < 25; k++) {
                if (k == seed % 10) {
                    break;
                }
                result += i * j * k;
                
                // Nested conditional
                if (result % 1000 == 0) {
                    result /= 2;
                } else {
                    result *= 2;
                }
            }
            j += (i % 3) + 1;
        }
        
        // Modify volatile
        seed++;
    }
    
    printf("Result: %d\n", result);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return result % 256;
}

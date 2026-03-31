#include <stdio.h>
#include <stdlib.h>

#define SIZE 100

volatile int g_volatile_counter = 0;
volatile int g_condition = 1;

int main() {
    int array[SIZE][SIZE];
    volatile int acc = 0;
    volatile int skip_inner = 0;
    
    // Initialize array with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = i * j + 1;
        }
    }
    
    // ========== COMPLEX NESTED LOOP STRUCTURE ==========
    // Outer loop - Level 1
    for (int i = 0; i < SIZE; i += 2) {
        // Multiple basic blocks in outer loop body
        if (g_condition) {
            // This block is in outer loop but NOT in middle loop
            acc += array[i][0];
            g_volatile_counter++;
            
            // Middle loop - Level 2 (not strictly contained in outer)
            // Starts in one branch, continues in another
            int j = 0;
            while (j < SIZE) {
                // Multiple basic blocks in middle loop
                if (j % 3 == 0) {
                    // Skip inner loop sometimes
                    skip_inner = 1;
                    acc += array[i][j];
                    j += 2;
                    continue;  // Creates additional basic block
                }
                
                // Inner loop - Level 3 (not strictly contained in middle)
                if (!skip_inner) {
                    // do-while for mixed loop type
                    int k = 0;
                    do {
                        // Complex inner loop body with multiple blocks
                        if (k % 2 == 0) {
                            acc += array[i][j] * k;
                            if (acc > 1000) {
                                // Early exit creates another block
                                acc = acc % 1000;
                            }
                        } else {
                            acc -= array[j][k] / 2;
                        }
                        
                        // Label and goto to create additional complexity
                        if (k == 5) {
                            goto special_case;
                        }
                        
                        k++;
                        
                        special_case:
                        if (k == 5) {
                            acc += 777;
                        }
                        
                    } while (k < 10 && g_volatile_counter < 500);
                }
                
                skip_inner = 0;
                j++;
                
                // Another basic block at end of while body
                if (j % 7 == 0) {
                    break;  // Creates exit block
                }
            }
        } else {
            // Alternative path in outer loop (not in middle loop)
            for (int x = 0; x < 5; x++) {
                acc -= x;
            }
        }
        
        // Outer loop continues with more blocks
        if (i % 4 == 0) {
            // Nested for loop with different bounds
            for (int y = 0; y < i && y < 20; y++) {
                acc += y * y;
                if (y == 10) {
                    // Another nested loop inside
                    for (int z = 0; z < 3; z++) {
                        acc += z;
                    }
                }
            }
        }
    }
    
    // ========== ADDITIONAL OVERLAPPING LOOP STRUCTURE ==========
    // Create another set of loops with partial overlap
    volatile int mode = 0;
    
    for (int a = 0; a < 50; a++) {
        // Loop A body - multiple blocks
        acc += a;
        
        if (mode) {
            // Loop B starts here but extends beyond A
            int b = a;
            while (b < 50) {
                acc += array[a][b];
                
                // Loop C inside B but not fully contained
                for (int c = 0; c < b && c < 10; c++) {
                    if (c % 2) {
                        acc -= c;
                        // Early continue creates block
                        if (acc < -100) continue;
                    }
                    acc += c * 2;
                }
                
                b += 1 + (a % 2);
                
                // B continues with blocks not in C
                if (b > 30) {
                    acc /= 2;
                }
            }
            // B ends here
        }
        
        // A continues with blocks not in B
        if (a % 3 == 0) {
            mode = !mode;
        }
    }
    
    // ========== FINAL COMPUTATION AND OUTPUT ==========
    printf("Result: %d\n", acc);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    // Use result to prevent optimization
    if (acc > 1000000) {
        return 1;
    }
    return 0;
}

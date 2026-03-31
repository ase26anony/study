#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define CHUNK 20

volatile int g_control = 0;
volatile int g_counter = 0;
volatile int g_modifier = 7;

int main() {
    int data[SIZE][SIZE];
    volatile int result = 0;
    
    // Initialize array with pattern
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            data[i][j] = (i * 31 + j * 17) % 256;
        }
    }
    
    // Outer loop - Level 1
    for (int outer = 0; outer < SIZE; outer += CHUNK) {
        // This if creates a basic block that's in outer but not necessarily in middle
        if (outer % 3 == 0) {
            g_control = 1;
            result += data[outer][0];
        } else {
            g_control = 0;
            result -= data[outer][0];
        }
        
        // Middle loop - Level 2 (not strictly contained in outer due to the if above)
        // This loop starts in one branch but continues in shared blocks
        int middle = 0;
        while (middle < SIZE) {
            // Multiple basic blocks within middle loop body
            if (middle % 2 == 0) {
                // Inner loop - Level 3 (not strictly contained in middle)
                for (int inner = 0; inner < CHUNK; inner++) {
                    // Complex body with multiple basic blocks
                    if (inner % g_modifier == 0) {
                        result += data[outer][middle] * inner;
                        // Early continue creates another basic block
                        if (inner == g_modifier) continue;
                    } else {
                        result -= data[outer][middle] / (inner + 1);
                    }
                    
                    // Another conditional inside inner loop
                    volatile int temp = inner * middle;
                    if (temp > 100) {
                        result >>= 1;
                        // Break could exit, creating more blocks
                        if (temp > 1000) break;
                    }
                    
                    g_counter++;
                }
                
                // Label and goto to create additional control flow
                if (middle > SIZE/2) {
                    goto skip_section;
                }
            } else {
                // Alternative path in middle loop
                result ^= data[outer][middle];
                
                // Do-while loop inside else branch (different type)
                int k = 0;
                do {
                    result += k * g_modifier;
                    k++;
                    if (k > 5) {
                        // Nested if with break
                        break;
                    }
                } while (k < 10);
            }
            
            skip_section:
            middle += (g_control ? 2 : 1);
            
            // Continue condition with volatile
            if (g_counter > 1000) {
                // Early exit from middle loop
                break;
            }
        }
        
        // Another loop type in outer (for with complex condition)
        for (int extra = outer; extra < outer + CHUNK && extra < SIZE; extra++) {
            // This loop shares some blocks with outer but not all
            result += data[extra][extra % CHUNK];
            
            // Conditional continue
            if (extra % 4 == 0) continue;
            
            // Small inner loop that partially overlaps
            int quick = 0;
            while (quick < 3) {
                result ^= (extra * quick);
                quick++;
                if (quick == 2 && g_control) {
                    // This creates partial block overlap
                    goto partial_exit;
                }
            }
            partial_exit:
            result++;
        }
    }
    
    // Final computation with all loops
    volatile int final_check = 0;
    int last = 0;
    
    // One more complex nested structure
    for (int a = 0; a < 50; a++) {
        for (int b = a; b < 50; b += (a + 1)) {
            // This inner loop is entered conditionally
            if (b % 3 == a % 3) {
                int c = 0;
                while (c < b) {
                    final_check += data[a % SIZE][b % SIZE] * c;
                    c += (g_modifier % 3) + 1;
                    
                    // Nested if with label
                    if (c > 20) {
                        goto inner_escape;
                    }
                }
                inner_escape:
                last = b;
            } else {
                // Different path that doesn't enter the while loop
                final_check -= data[b % SIZE][a % SIZE];
            }
        }
    }
    
    printf("Result: %d, Final: %d, Counter: %d\n", 
           result, final_check, g_counter);
    
    return (result + final_check + g_counter) % 256;
}

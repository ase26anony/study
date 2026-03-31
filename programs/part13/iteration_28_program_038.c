#include <stdio.h>
#include <stdlib.h>

// Volatile variables to prevent optimization
volatile int g_outer_cond = 1;
volatile int g_middle_cond = 2;
volatile int g_inner_cond = 3;
volatile int g_accumulator = 0;
volatile int g_switch = 0;

// Function to create more complex control flow
int process_value(int x, int y, int z) {
    volatile int result = 0;
    
    if (x % 2) {
        result += y * z;
        if (y > z) {
            result -= x;
        } else {
            result += x;
            // Early return in some cases
            if (z % 3 == 0) return result;
        }
    } else {
        result = y + z;
        // Nested condition
        if (z % 4 == 0) {
            result *= 2;
        }
    }
    
    // Multiple basic blocks
    switch (x % 4) {
        case 0: result += 1; break;
        case 1: result += 2; break;
        case 2: result += 3; break;
        case 3: result += 4; break;
    }
    
    return result;
}

int main() {
    // Multi-dimensional arrays
    int array1[10][10][10];
    int array2[10][10];
    volatile int temp = 0;
    
    // Initialize arrays
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            array2[i][j] = i * j;
            for (int k = 0; k < 10; k++) {
                array1[i][j][k] = i + j + k;
            }
        }
    }
    
    // ========== COMPLEX NESTED LOOPS WITH PARTIAL OVERLAP ==========
    
    // Outer loop - fixed iteration count
    for (int outer = 0; outer < 8; outer++) {
        // Multiple basic blocks in outer loop body
        g_accumulator += outer;
        
        // Conditional that sometimes skips inner structures
        if (outer % g_outer_cond != 0) {
            // This block is in outer but not in middle/inner
            
            // Middle loop - NOT strictly contained in outer
            // because it has blocks outside the if condition
            int middle_start = (outer % 2) ? 0 : 2;
            int middle_end = (outer % 3) ? 6 : 4;
            
            // Middle loop with variable bounds
            for (int middle = middle_start; middle < middle_end; middle++) {
                // Multiple basic blocks in middle loop
                int val = process_value(outer, middle, 0);
                
                // Inner loop - NOT strictly contained in middle
                // because of the conditional entry
                if (middle % g_middle_cond != 0) {
                    // Inner loop with mixed type
                    int inner = 0;
                    do {
                        // Complex inner loop body with break/continue
                        if (inner % 5 == 0) {
                            continue;  // Creates additional basic block
                        }
                        
                        g_accumulator += array1[outer][middle][inner % 10];
                        
                        // Conditional break
                        if (inner > g_inner_cond && (inner % 7 == 0)) {
                            break;
                        }
                        
                        inner++;
                    } while (inner < (middle % 4 + 3));
                    
                    // Block after do-while (in middle but not in inner)
                    temp += val;
                } else {
                    // Alternative path in middle loop (not containing inner)
                    temp -= val;
                    // Goto to create additional control flow
                    if (val > 10) goto skip_point;
                }
                
                // Label for goto
                skip_point:
                // Empty but creates a basic block
                ;
            }
            
            // Block after middle loop (in outer but not in middle)
            g_switch = !g_switch;
        } else {
            // Alternative outer path (no middle/inner loops)
            g_accumulator *= 2;
        }
        
        // Continue in outer loop
        if (outer % 4 == 0) {
            continue;
        }
        
        // Additional computation in outer loop
        temp += array2[outer % 10][0];
    }
    
    // ========== SECOND SET: DIFFERENT NESTING PATTERN ==========
    
    // While loop as outermost
    int counter = 0;
    volatile int limit = 5;
    
    while (counter < limit) {
        // For loop inside while
        for (int i = 0; i < 3 + counter; i++) {
            // Conditional with loop inside one branch
            if (i % 2 == g_switch) {
                // Innermost loop with partial overlap
                for (int j = i; j < 5; j++) {
                    // Multiple blocks with if-else
                    if (j % 2) {
                        g_accumulator += j;
                    } else {
                        g_accumulator -= j;
                        // Nested if
                        if (j == 3) {
                            g_accumulator *= 2;
                        }
                    }
                }
            } else {
                // Different computation path
                g_accumulator += i * 10;
            }
            
            // Break from middle loop under condition
            if (g_accumulator > 1000) {
                break;
            }
        }
        
        counter++;
        // Modify condition
        if (counter == 3) {
            limit = 8;  // Extend loop
        }
    }
    
    // ========== THIRD SET: THREE-LEVEL MIXED NESTING ==========
    
    // Outer do-while
    int a = 0;
    do {
        // Middle for with complex condition
        for (int b = a; b < 4; b += 1 + (a % 2)) {
            // Inner while with volatile condition
            int c = 0;
            volatile int inner_limit = 2 + (b % 3);
            
            while (c < inner_limit) {
                // Complex body with switch
                switch ((a + b + c) % 4) {
                    case 0:
                        temp += array1[a % 10][b % 10][c % 10];
                        // Fall through
                    case 1:
                        temp -= 1;
                        break;
                    case 2:
                        // Nested if in switch
                        if (temp > 50) {
                            temp /= 2;
                        } else {
                            temp *= 2;
                        }
                        break;
                    default:
                        temp = temp ^ (a * b * c);
                }
                
                c++;
                // Continue under condition
                if (c % 2 == 0) continue;
                
                // Additional computation
                g_accumulator += temp % 100;
            }
            
            // Block after while (in for but not in while)
            if (b % 2 == 0) {
                // Another conditional with goto
                if (temp < 0) goto adjust;
            }
            
            adjust:
            temp = abs(temp) % 100;
        }
        
        a++;
    } while (a < 3);
    
    // Final output to prevent elimination
    printf("Result: %d (accumulator: %d, temp: %d)\n", 
           g_accumulator + temp, g_accumulator, temp);
    
    return (g_accumulator + temp) % 100;
}

#include <stdio.h>
#include <stdlib.h>

// Volatile variables to prevent optimization
volatile int g_outer_cond = 1;
volatile int g_middle_cond = 1;
volatile int g_inner_cond = 1;
volatile int g_counter = 0;
volatile int g_result = 0;

// Complex loop structure with partial overlaps
void complex_nested_loops(int size) {
    volatile int i, j, k;
    volatile int temp = 0;
    
    // Outer loop - level 1
    for (i = 0; i < size; i++) {
        // Multiple basic blocks in outer loop body
        if (g_outer_cond) {
            // First basic block in outer loop
            temp += i * 2;
            
            // Conditional that creates partial overlap
            if (i % 3 == 0) {
                // Middle loop - level 2 (not fully contained in outer)
                // This loop starts here but continues outside this if block
                j = 0;
                while (j < size / 2) {
                    // Multiple basic blocks in middle loop
                    if (j % 2 == 0) {
                        temp += j * 3;
                        
                        // Inner loop - level 3 (not fully contained in middle)
                        // This creates the partial overlap scenario
                        for (k = 0; k < size / 3; k++) {
                            // Complex inner loop body with multiple blocks
                            if (k % 4 == 0) {
                                temp += k * 4;
                                // Early continue creates additional basic block
                                if (k == size / 6) continue;
                                temp -= 1;
                            } else {
                                temp += 1;
                                // Break statement creates exit block
                                if (k == size / 4) break;
                            }
                            
                            // Another basic block in inner loop
                            temp += (i + j + k) % 5;
                        }
                        
                        // Label and goto to create additional control flow
                        mid_loop_tail:
                        temp += j * j;
                    } else {
                        // Alternative path in middle loop
                        temp -= j;
                        // Skip inner loop sometimes
                        if (j == size / 4) goto mid_loop_tail;
                    }
                    
                    // Middle loop increment with condition
                    j++;
                    if (j == size / 3) {
                        // Additional basic block
                        temp += 100;
                    }
                }
                
                // Code after middle loop but still in outer loop's if block
                temp += i * i;
            } else {
                // Alternative path in outer loop - no middle loop here
                // This creates the partial block overlap
                temp -= i;
                
                // Different loop structure in this branch
                j = size - 1;
                do {
                    temp += j * 2;
                    j--;
                } while (j > 0 && g_middle_cond);
            }
        } else {
            // Another alternative path with different loop
            for (j = size / 2; j < size; j++) {
                temp += j * 5;
            }
        }
        
        // Common tail for outer loop
        temp %= 1000;
        
        // Nested switch to create more basic blocks
        switch (i % 4) {
            case 0:
                temp += 7;
                break;
            case 1:
                temp += 11;
                // Fall through
            case 2:
                temp += 13;
                break;
            default:
                temp += 17;
        }
    }
    
    g_result = temp;
}

// Additional complex loop structure with mixed types
void mixed_loop_types(int size) {
    volatile int a, b, c;
    volatile int acc = 0;
    
    // Fixed iteration for loop
    for (a = 0; a < size; a++) {
        // Conditional with partial loop
        if (a % 2 == 0) {
            // While loop with variable condition
            b = 0;
            while (b < size / 2 && g_inner_cond) {
                // Do-while inside while
                c = 0;
                do {
                    acc += (a + b + c) % 7;
                    c++;
                    
                    // Conditional break
                    if (c == size / 3 && a % 3 == 0) {
                        acc += 50;
                        break;
                    }
                    
                    // Continue with condition
                    if (c % 5 == 0) {
                        acc -= 10;
                        continue;
                    }
                    
                    acc += 1;
                } while (c < size / 4);
                
                b++;
                
                // Additional control flow
                if (b == size / 5) {
                    acc *= 2;
                    // Goto to create non-trivial CFG
                    if (acc > 1000) goto while_exit;
                }
            }
            while_exit:
            acc %= 500;
        } else {
            // Different loop structure for odd a
            for (b = size - 1; b >= 0; b -= 2) {
                acc += b * a;
                
                // Nested with partial containment
                if (b % 3 == 0) {
                    c = 0;
                    while (c < b && g_counter) {
                        acc += c;
                        c += 2;
                    }
                }
            }
        }
        
        // Loop with early exit
        for (b = 0; b < size; b++) {
            acc += b;
            if (acc > 10000) {
                acc = 10000;
                break;
            }
        }
    }
    
    g_result += acc;
}

int main() {
    int size = 50;
    
    // Call both complex loop functions
    complex_nested_loops(size);
    mixed_loop_types(size);
    
    // Use volatile to prevent dead code elimination
    volatile int final_result = g_result;
    
    // Print to ensure code isn't optimized away
    printf("Result: %d\n", final_result);
    
    // Additional complex nesting
    volatile int x, y, z;
    volatile int sum = 0;
    
    // Triple nested loops with complex conditions
    for (x = 0; x < 20; x++) {
        if (x % 3 != 0) {
            y = x;
            while (y < 30) {
                // Partial overlap: this block is in both outer and middle
                sum += x * y;
                
                for (z = 0; z < 15; z++) {
                    // Not fully contained in middle loop
                    if (z % 2 == 0) {
                        sum += z;
                        if (z == 10) {
                            // Early exit creates additional block
                            sum += 100;
                            continue;
                        }
                    } else {
                        sum -= z;
                    }
                    
                    // Code that's only in inner loop
                    if (y > 15) {
                        sum += 50;
                    }
                }
                
                y += 2;
                
                // Code after inner loop but still in middle
                if (y % 5 == 0) {
                    sum += 25;
                }
            }
        } else {
            // Alternative path - different loop structure
            for (y = 0; y < 10; y++) {
                sum += y * 3;
            }
        }
        
        // Common tail with another loop
        z = 0;
        do {
            sum += z;
            z++;
        } while (z < 5 && x % 2 == 0);
    }
    
    printf("Final sum: %d\n", sum + final_result);
    
    return (sum + final_result) % 256;
}

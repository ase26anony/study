#include <stdio.h>
#include <stdlib.h>

// Volatile variables to prevent optimization
volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;
volatile int g_inner_skip = 1;

// Complex loop structure with partial overlaps
void complex_nested_loops(int size) {
    volatile int accumulator = 0;
    int arr[100][100];
    
    // Initialize array
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    // OUTER LOOP - Level 1
    // This loop has multiple basic blocks due to internal conditions
    for (int i = 0; i < size; i++) {
        // Basic block A in outer loop
        accumulator += arr[i][0];
        
        // Conditional that creates partial overlap
        if (g_outer_skip || (i % 3 == 0)) {
            // Basic block B (sometimes skipped)
            accumulator -= arr[0][i];
            
            // MIDDLE LOOP - Level 2
            // Not strictly contained in outer loop due to the condition
            int j = 0;
            while (j < size - i) {  // while loop for variety
                // Basic block C in middle loop
                accumulator += arr[i][j];
                
                // Nested condition inside middle loop
                if (j % 2 == 0) {
                    // Basic block D
                    accumulator *= 2;
                    
                    // INNER LOOP - Level 3
                    // for loop with constant bounds
                    for (int k = 0; k < 10; k++) {
                        // Basic block E in inner loop
                        accumulator += arr[j][k];
                        
                        // Internal if-else in inner loop body
                        if (k % 3 == 0) {
                            // Basic block F
                            accumulator -= 1;
                            // continue statement creates additional control flow
                            if (accumulator > 1000) continue;
                        } else {
                            // Basic block G
                            accumulator += 2;
                            // break can exit early
                            if (accumulator < -1000) break;
                        }
                        
                        // Basic block H (shared)
                        accumulator ^= 0xFF;
                    } // end inner for loop
                    
                    // Label and goto for additional complexity
                    if (accumulator > 5000) {
                        goto skip_point;
                    }
                } else {
                    // Basic block I (alternative path)
                    accumulator /= 2;
                }
                
                // Basic block J (merge point)
                j += (accumulator % 2) + 1;  // Variable increment
                
                skip_point:
                // Empty label target
                g_volatile_counter++;
            } // end middle while loop
            
            // Basic block K (after middle loop)
            if (g_inner_skip) {
                accumulator |= 0xAAAA;
            }
        } else {
            // Basic block L (alternative outer path)
            // Another loop that doesn't contain the middle loop
            do {
                accumulator >>= 1;
                g_volatile_counter--;
            } while (accumulator > 100 && g_volatile_counter < 50);
        }
        
        // Basic block M (outer loop continuation)
        // Another conditional with goto
        if (accumulator < 0) {
            goto outer_continue;
        }
        
        // Basic block N
        accumulator = accumulator % 1000;
        
        outer_continue:
        // Empty label
        ;
    } // end outer for loop
    
    // Use accumulator to prevent dead code elimination
    printf("Result: %d\n", accumulator);
}

// Additional complex loop structure with different patterns
void overlapping_loops_pattern2(int n) {
    volatile int sum = 0;
    int matrix[50][50];
    
    // Initialize
    for (int x = 0; x < 50; x++) {
        for (int y = 0; y < 50; y++) {
            matrix[x][y] = x * y;
        }
    }
    
    // Loop A - outer but not fully containing others
    for (int a = 0; a < n; a++) {
        sum += matrix[a][0];
        
        // Conditional branch determining loop nesting
        if (a % 4 != 0) {
            // Loop B - partially overlaps with A
            int b = a;
            while (b < n && b < 30) {
                sum -= matrix[0][b];
                
                // Loop C - partially overlaps with B
                for (int c = 0; c < 15; c++) {
                    sum += matrix[b][c];
                    
                    // Internal control flow
                    switch (c % 3) {
                        case 0: sum <<= 1; break;
                        case 1: sum >>= 1; break;
                        case 2: sum ^= matrix[c][b]; break;
                    }
                    
                    if (sum > 10000) goto early_exit;
                }
                
                b += (sum % 3) + 1;
                
                early_exit:
                // Target for goto
                if (b > 25) break;
            }
        } else {
            // Alternative path with different loop
            for (int d = 10; d > 0; d--) {
                sum |= matrix[d][a];
            }
        }
        
        // Another loop that sometimes executes
        if (sum % 7 == 0) {
            int e = 5;
            do {
                sum &= 0xFF;
                e--;
            } while (e > 0 && sum < 5000);
        }
    }
    
    printf("Pattern2 sum: %d\n", sum);
}

int main() {
    int size = 20;
    
    // Modify volatile variables to affect control flow
    g_outer_skip = (size % 2 == 0) ? 1 : 0;
    g_inner_skip = 0;
    
    // Execute complex nested loops
    complex_nested_loops(size);
    
    // Change volatile state
    g_outer_skip = 1;
    g_inner_skip = 1;
    
    // Execute second pattern
    overlapping_loops_pattern2(size);
    
    // Return value based on computations
    return g_volatile_counter > 0 ? 0 : 1;
}

#include <stdio.h>
#include <stdlib.h>

// Volatile variables to prevent optimization
volatile int g_volatile_counter = 0;
volatile int g_outer_skip = 0;
volatile int g_inner_skip = 1;

// Complex loop structure with partial block overlap
void complex_nested_loops(int size) {
    volatile int accumulator = 0;
    int i, j, k;
    
    // Outer loop - Level 1
    for (i = 0; i < size; i++) {
        // Multiple basic blocks in outer loop body
        if (g_outer_skip) {
            // This block is in outer loop but NOT in middle loop
            accumulator += i * 2;
            continue;  // Creates additional basic block
        }
        
        // Middle loop - Level 2 (not strictly contained in outer)
        // This loop starts in outer but has blocks outside outer's body
        j = 0;
        while (j < size) {
            // Multiple basic blocks in middle loop
            if (j % 3 == 0) {
                accumulator -= j;
                j += 2;  // Skip some iterations
                continue;
            }
            
            // Conditional that sometimes skips inner loop
            if (g_inner_skip && (j % 5 == 0)) {
                // Block in middle but NOT in inner loop
                accumulator += i * j;
                j++;
                continue;
            }
            
            // Inner loop - Level 3 (not strictly contained in middle)
            // Fixed iteration count to encourage hardware loops
            for (k = 0; k < 10; k++) {
                // Complex body with multiple blocks
                if (k % 2 == 0) {
                    accumulator += i + j + k;
                    if (accumulator > 1000) {
                        // Early exit creates another block
                        accumulator %= 1000;
                    }
                } else {
                    accumulator -= k;
                }
                
                // Label and goto to create additional blocks
                if (k == 5) {
                    goto special_case;
                }
                continue;
                
            special_case:
                accumulator += 100;
            }
            
            // Post-inner loop block (in middle but not in inner)
            if (accumulator < 0) {
                accumulator = 0;
            }
            
            j++;
        }
        
        // Another block in outer but not in middle
        if (i % 7 == 0) {
            do {
                // Do-while creates different loop structure
                accumulator++;
                g_volatile_counter++;
            } while (accumulator % 3 != 0);
        }
    }
    
    // Prevent dead code elimination
    printf("Accumulator: %d\n", accumulator);
}

// Additional nested structure with different patterns
void mixed_loop_types(int n) {
    volatile int sum = 0;
    int a = 0, b = 0, c = 0;
    
    // Outer for loop
    for (a = 0; a < n; a++) {
        // Conditional with partial overlap
        if (a % 2 == 0) {
            // Middle while loop - partially overlapping
            b = a;
            while (b < n) {
                // Multiple blocks in while body
                sum += b;
                
                if (b % 4 == 0) {
                    // Inner do-while - not fully contained
                    c = 0;
                    do {
                        sum -= c;
                        c++;
                        if (c > 5) break;  // Creates break block
                    } while (c < 8 && sum < 1000);
                    
                    // Block after do-while (in while but not in do-while)
                    sum += 10;
                } else {
                    // Alternative path without inner loop
                    sum += 20;
                }
                
                // Another conditional in while
                if (sum > 500) {
                    sum = sum / 2;
                    continue;  // Creates continue block
                }
                
                b += 1 + (a % 3);
            }
        } else {
            // Different path in outer loop
            sum += a * 3;
        }
        
        // Goto label to create additional basic block
        if (a == n / 2) {
            goto midpoint;
        }
        continue;
        
    midpoint:
        sum += 1000;
    }
    
    printf("Mixed sum: %d\n", sum);
}

int main() {
    int size = 50;
    
    // Call both complex loop structures
    complex_nested_loops(size);
    mixed_loop_types(size);
    
    // Additional volatile operations to prevent optimization
    g_volatile_counter += size;
    
    // Return value based on computations
    return g_volatile_counter > 0 ? 0 : 1;
}

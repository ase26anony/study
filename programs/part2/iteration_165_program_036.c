#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 16; // volatile to prevent constant propagation
    int acc1 = 0, acc2 = 0;
    
    // Loop A: Will share some blocks with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            acc1 += i * 2;
            continue; // Creates separate basic block
        }
        acc1 += i;
        
        // Loop B: Sequential loop that shares some blocks with Loop A
        // This creates partial bitmap overlap
        if (i == seed % 8) {
            int j = 0;
            while (j < 8) {
                acc2 += (i * j);
                if (j % 2 == 0) {
                    j += 2;
                    continue;
                }
                j++;
            }
        }
    }
    
    // Loop C: Another sequential loop that shares exit block with Loop A
    for (int k = 0; k < limit; ++k) {
        switch (k % 4) {
            case 0: acc1 += k; break;
            case 1: acc2 += k * 3; break;
            case 2: acc1 -= k; break;
            default: acc2 -= k; break;
        }
    }
    
    return acc1 + acc2;
}

__attribute__((noinline))
int nested_loops_2(int base) {
    int total = 0;
    volatile int outer_limit = 12;
    
    // Outer loop D: Contains inner loops as strict subsets
    for (int x = 0; x < outer_limit; ++x) {
        if (x < base) {
            // Inner loop E: Strict subset of outer loop's blocks
            for (int y = 0; y < 8; ++y) {
                total += x * y;
                if (y == 4) break; // Early exit creates separate block
            }
        } else {
            // Inner loop F: Another subset with different structure
            int z = 0;
            do {
                total -= x + z;
                z++;
                if (z > 3) continue; // Branch back to condition
            } while (z < 6);
        }
        
        // Conditional inner loop G: May not execute
        if (x % 3 == 1) {
            for (int w = 0; w < 4; ++w) {
                total += w * w;
                // Function call creates separate block
                if (w == 2) {
                    total += complex_loops_1(w); // Recursive call
                }
            }
        }
    }
    
    return total;
}

__attribute__((noinline))
int overlapping_loops_3(int init) {
    int sum1 = init, sum2 = init;
    volatile int n = 10;
    
    // Loop H and Loop I: Sequential loops with shared pre-header
    // but different bodies - partial overlap
    
    // Loop H
    int i = 0;
    while (i < n) {
        sum1 += i * i;
        if (i % 2 == 0) {
            i += 2;
        } else {
            i++;
        }
    }
    
    // Shared basic block here (after Loop H, before Loop I)
    
    // Loop I: Shares some blocks with Loop H but not all
    for (int j = 0; j < n; ++j) {
        sum2 += j * 3;
        if (j == n/2) {
            // Nested loop J: Creates more complex bitmap
            for (int k = 0; k < 5; ++k) {
                sum1 -= k;
                if (k == 3) continue;
                sum2 += k;
            }
        }
    }
    
    // Loop K: Intersects with both H and I incompletely
    do {
        sum1 = sum1 * 2 - sum2;
        n--; // Modifying volatile-bound variable
    } while (n > 0);
    
    return sum1 + sum2;
}

int main() {
    // Initialize with non-constant to prevent compile-time evaluation
    int seed = 7;
    int result = 0;
    
    // Execute all loop patterns to ensure analysis
    result += complex_loops_1(seed);
    result += nested_loops_2(seed);
    result += overlapping_loops_3(seed);
    
    // Create data dependency between calls
    seed = result % 100;
    result += complex_loops_1(seed);
    
    // Final observable output
    printf("Result checksum: %d\n", result);
    
    return 0;
}

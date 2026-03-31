#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 16; // volatile to prevent optimization
    int sum = 0;
    
    // Loop A: Sequential loop that will share blocks with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        // Loop B: Nested inside Loop A's block but with different control
        for (int j = 0; j < 8; ++j) {
            if (j % 2 == 0) {
                sum += j * seed;
            } else {
                // Early continue creates additional basic blocks
                if (j == 5) continue;
                sum += j;
            }
        }
        
        // Early break creates divergent control flow
        if (sum > 1000) break;
    }
    
    return sum;
}

__attribute__((noinline))
int overlapping_loops_2(int base) {
    int acc1 = 0, acc2 = 0;
    volatile int bound = 24;
    
    // Loop C and Loop D are sequential at same nesting level
    // They will share some common basic blocks (setup, post-loop)
    
    // Loop C
    for (int x = 0; x < bound; ++x) {
        switch (x % 4) {
            case 0: acc1 += x * 3; break;
            case 1: acc1 += x * 2; break;
            case 2: acc1 += x; break;
            default: acc1 += 1; break;
        }
        
        // Nested loop E inside C (subset of C's blocks)
        for (int y = 0; y < 4; ++y) {
            acc1 += y * base;
            if (y == 2) continue;
            acc1 -= 1;
        }
    }
    
    // Shared post-loop block between C and D
    
    // Loop D - sequential to C, partially overlapping blocks
    for (int x = 5; x < bound - 3; ++x) {
        if (x % 2 == 0) {
            acc2 += x * x;
            // Another nested loop F
            for (int z = 0; z < 3; ++z) {
                acc2 += z * base;
                if (z == 1) break;
            }
        } else {
            acc2 += x;
        }
    }
    
    return acc1 + acc2;
}

__attribute__((noinline))
int nested_subset_loops_3(int init) {
    int total = init;
    volatile int outer_limit = 12;
    
    // Loop G: Outer loop containing multiple inner loops
    for (int a = 0; a < outer_limit; ++a) {
        total += a;
        
        // Loop H: Strict subset of G's blocks
        for (int b = 0; b < 6; ++b) {
            total += b * 2;
            if (b == 3) continue;
            total -= 1;
        }
        
        // Loop I: Another subset with different structure
        int c = 0;
        do {
            total += c * a;
            c++;
            if (c > 4) break;
        } while (c < 8);
        
        // Conditional early exit creates more block variations
        if (total > 5000) {
            // Loop J: Only executed sometimes
            for (int d = 0; d < 2; ++d) {
                total += d * 100;
            }
            break;
        }
    }
    
    return total;
}

__attribute__((noinline))
int interleaved_control_flow(int start) {
    int result = start;
    volatile int mod = 7;
    
    // Multiple loops with interleaved control flow
    // Loop K
    for (int i = 0; i < 20; i += 2) {
        result += i;
        
        // Loop L: Sometimes executed
        if (i % mod == 0) {
            for (int j = 0; j < 5; ++j) {
                result += j * i;
                if (j == 3) continue;
                result -= 2;
            }
        }
    }
    
    // Loop M: Sequential with partial block overlap
    int k = 0;
    while (k < 15) {
        result += k * 3;
        
        // Loop N: Always nested, strict subset
        for (int n = 0; n < 3; ++n) {
            result += n + k;
        }
        
        k++;
        if (k == 10) {
            // Additional block only in this path
            result += 100;
        }
    }
    
    return result;
}

int main() {
    int seed = 1;
    int checksum = 0;
    
    // Force data dependencies between loops
    checksum = complex_loops_1(seed);
    checksum = overlapping_loops_2(checksum);
    checksum = nested_subset_loops_3(checksum);
    checksum = interleaved_control_flow(checksum);
    
    // Final observable output
    printf("Final checksum: %d\n", checksum);
    
    // Additional volatile operations to prevent dead code elimination
    volatile int dummy = checksum;
    if (dummy > 0) {
        printf("Positive result\n");
    }
    
    return checksum > 0 ? 0 : 1;
}

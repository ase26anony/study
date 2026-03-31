#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(volatile int seed) {
    int sum = 0;
    volatile int limit = seed + 10;
    
    // Loop A: Sequential loop with complex control flow
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
            continue;
        }
        if (i % 5 == 0) {
            sum -= i;
            break;  // Early exit creates different basic blocks
        }
        sum += i;
    }
    
    // Loop B: Sequential loop that shares some blocks with Loop A's structure
    // This creates partial bitmap overlap
    for (int j = 0; j < limit; ++j) {
        switch (j % 4) {
            case 0:
                sum += j * 3;
                break;
            case 1:
                sum -= j;
                // Fall through to create shared block patterns
            case 2:
                sum += j / 2;
                break;
            default:
                if (j > 5) {
                    sum += 1;
                }
                break;
        }
    }
    
    return sum;
}

__attribute__((noinline))
int nested_loops_pattern(volatile int base) {
    int total = 0;
    volatile int outer_limit = base % 8 + 5;
    volatile int inner_limit = 8;  // Fixed for hardware loop candidate
    
    // Outer loop with inner loops that are subsets
    for (int x = 0; x < outer_limit; ++x) {
        total += x * x;
        
        // Inner loop 1: Strict subset of outer loop's blocks
        // Should trigger bitmap_intersect_compl_p checks
        for (int y = 0; y < inner_limit; ++y) {
            total += y + x;
            if (y == 3) {
                total -= 1;  // Conditional creates additional blocks
            }
        }
        
        // Inner loop 2: Different structure but overlapping
        int z = 0;
        while (z < inner_limit) {
            total += z * x;
            if (z % 2 == 0) {
                total += 2;
                z += 2;
            } else {
                z += 1;
            }
        }
        
        // Conditional that creates shared post-loop blocks
        if (x % 2 == 0) {
            total += 100;
        }
    }
    
    return total;
}

__attribute__((noinline))
int hardware_loop_candidates(volatile int iter) {
    int acc = 0;
    
    // Perfect hardware loop candidate: fixed, moderate iteration count
    for (int i = 0; i < 32; ++i) {
        acc += i * i;
        // Simple arithmetic prevents dead code elimination
    }
    
    // Another candidate with different block structure
    for (int j = 16; j > 0; --j) {
        acc += j;
        if (j == 8) {
            acc *= 2;  // Conditional creates different bitmap
        }
    }
    
    // Do-while loop for variety
    int k = 0;
    do {
        acc += k * 3;
        k++;
    } while (k < iter % 10 + 5);
    
    return acc;
}

__attribute__((noinline))
int overlapping_control_flow(volatile int param) {
    int result = param;
    
    // Multiple loops at same level with shared header-like structure
    volatile int bound = param % 7 + 3;
    
    // Loop X
    for (int a = 0; a < bound; a++) {
        result += a;
        if (a == bound - 1) {
            result *= 2;  // Creates shared exit block pattern
        }
    }
    
    // Loop Y - similar but not identical structure
    // Will have partial block overlap with Loop X
    for (int b = 0; b < bound; b++) {
        result -= b;
        switch (b) {
            case 0: result += 5; break;
            case 1: result += 10; break;
            default: result += 1; break;
        }
    }
    
    // Loop Z - different bounds but overlapping blocks
    for (int c = bound; c > 0; c--) {
        result += c * c;
        if (c % 2 == 0) {
            continue;  // Creates different control flow
        }
        result += 7;
    }
    
    return result;
}

int main() {
    volatile int seed = 42;  // Volatile to prevent constant propagation
    int final_sum = 0;
    
    // Execute all loop patterns to ensure analysis
    final_sum += complex_loops_1(seed);
    final_sum += nested_loops_pattern(seed);
    final_sum += hardware_loop_candidates(seed);
    final_sum += overlapping_control_flow(seed);
    
    // Additional sequential loops in main to create more opportunities
    for (int m = 0; m < 20; m++) {
        final_sum += m;
        // Nested loop inside - subset relationship
        for (int n = 0; n < 5; n++) {
            final_sum -= n;
        }
    }
    
    // Another loop pair with partial overlap
    int temp = 0;
    for (int p = 0; p < 15; p++) {
        temp += p * p;
    }
    for (int q = 5; q < 20; q++) {
        temp += q;  // Different bounds create partial overlap
    }
    final_sum += temp;
    
    // Print result to prevent dead code elimination
    printf("Final checksum: %d\n", final_sum);
    
    return final_sum != 0 ? 0 : 1;
}

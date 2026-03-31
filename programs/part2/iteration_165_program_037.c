#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 16; // volatile to prevent optimization
    int sum = 0;
    
    // Loop A: Will have blocks that partially overlap with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
            // Early continue creates additional basic block
            continue;
        }
        // Common block shared with Loop B
        sum += i;
        if (i == limit - 1) {
            // Break creates exit block
            break;
        }
    }
    
    // Loop B: Sequential loop sharing some blocks with Loop A
    int j = 0;
    while (j < limit) {
        // Shared arithmetic pattern with Loop A
        sum += j;
        if (j % 2 == 0) {
            sum += j * 3;
            // Function call creates complex block
            sum += abs(j - 5);
        }
        j++;
    }
    
    return sum + seed;
}

__attribute__((noinline))
int nested_loops_2(int base) {
    volatile int outer_limit = 8;
    volatile int inner_limit = 4;
    int acc = base;
    
    // Outer loop: blocks will be superset of inner loop blocks
    for (int x = 0; x < outer_limit; ++x) {
        acc += x;
        
        // Inner loop 1: Strict subset of outer loop blocks
        for (int y = 0; y < inner_limit; ++y) {
            acc += x * y;
            if (y % 2 == 0) {
                acc += 1;
            }
        }
        
        // Conditional creates divergent paths
        if (x % 3 == 0) {
            // Another inner loop with different structure
            int z = 0;
            do {
                acc -= z;
                z++;
            } while (z < 3);
        } else {
            acc += 100;
        }
    }
    
    return acc;
}

__attribute__((noinline))
int overlapping_loops_3(int init) {
    int result = init;
    volatile int n = 10;
    
    // Loop C and D will have intersecting but not subset block bitmaps
    
    // Loop C with switch inside
    for (int i = 0; i < n; i++) {
        switch (i % 4) {
            case 0: result += i; break;
            case 1: result += i * 2; break;
            case 2: result += i * 3; 
                    // Continue creates additional edge
                    continue;
            default: result -= i;
        }
        // Common block that Loop D also uses
        result += 5;
    }
    
    // Loop D: Overlaps with Loop C but has different structure
    int k = n - 1;
    while (k >= 0) {
        result += k;
        // Shared block with Loop C
        result += 5;
        if (k < n / 2) {
            // Different block not in Loop C
            result *= 2;
            break; // Early exit creates different exit block
        }
        k--;
    }
    
    return result;
}

__attribute__((noinline))
int hardware_loop_candidate(int start) {
    int total = start;
    
    // Perfect candidate for hardware loop: fixed, moderate iteration count
    for (int i = 0; i < 32; ++i) {
        total += i * i;
        // Simple conditional that doesn't break loop structure
        if (i & 1) {
            total -= i;
        }
    }
    
    // Another hardware loop candidate at same nesting level
    for (int j = 0; j < 24; ++j) {
        total += j * 3;
        // Nested loop inside - creates subset relationship
        for (int k = 0; k < 8; ++k) {
            total += k;
        }
    }
    
    return total;
}

int main() {
    volatile int seed = 42; // volatile to prevent constant propagation
    
    // Execute all loop patterns to force analysis
    int r1 = complex_loops_1(seed);
    int r2 = nested_loops_2(r1);
    int r3 = overlapping_loops_3(r2);
    int r4 = hardware_loop_candidate(r3);
    
    // Create data dependencies between results
    int final_result = r1 + r2 * 2 + r3 / 3 + r4;
    
    // Print to ensure all computations are live
    printf("Final checksum: %d\n", final_result);
    
    // Additional volatile operations to prevent dead code elimination
    volatile int dummy = final_result;
    
    return final_result > 0 ? 0 : 1;
}

#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 16; // Volatile to prevent constant propagation
    int acc = seed;
    
    // Loop A: Will have blocks that partially overlap with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            acc += i * 2;
            // Early continue creates additional basic block
            continue;
        }
        acc += i;
        
        // Nested loop inside A - strict subset of A's blocks
        for (int j = 0; j < 8; ++j) {
            if (j & 1) {
                acc += j * 3;
            } else {
                acc -= j;
            }
            // Function call creates another basic block
            rand();
        }
        
        if (i == limit - 1) {
            break; // Early exit creates another block
        }
    }
    
    // Loop B: Sequential to A, shares some blocks (like function calls)
    // but not all - should cause partial overlap
    int temp = 0;
    for (int k = 0; k < limit * 2; ++k) {
        if (k < limit) {
            temp += k * acc;
            rand(); // Shared basic block with Loop A
        } else {
            temp -= k;
            // Different block structure than Loop A
            switch (k % 4) {
                case 0: temp += 1; break;
                case 1: temp += 2; break;
                case 2: temp += 3; break;
                default: temp += 4;
            }
        }
    }
    
    return acc + temp;
}

__attribute__((noinline))
int complex_loops_2(int base) {
    volatile int outer_limit = 12;
    volatile int inner_limit = 6;
    int result = base;
    
    // Loop C: Outer loop with complex structure
    for (int x = 0; x < outer_limit; ++x) {
        result ^= x;
        
        // Multiple inner loops at same level within outer loop
        // Inner Loop D: Strict subset of C's blocks
        for (int y = 0; y < inner_limit; ++y) {
            result += y * x;
            if (y == inner_limit - 1) {
                result *= 2;
            }
        }
        
        // Inner Loop E: Also subset of C, but different from D
        for (int z = 0; z < inner_limit / 2; ++z) {
            result -= z + x;
            rand(); // Shared with other loops
        }
        
        // Conditional block that's part of C but not in D or E
        if (x % 2 == 0) {
            result >>= 1;
        } else {
            result <<= 1;
        }
    }
    
    // Loop F: Sequential to C, partially overlapping blocks
    // but not a subset (has blocks C doesn't have)
    int counter = 0;
    do {
        result += counter++;
        if (counter > 8) {
            // Different control flow than previous loops
            for (int i = 0; i < 3; ++i) {
                result += rand() % 10;
            }
            break;
        }
        rand(); // Shared basic block
    } while (counter < 10);
    
    return result;
}

__attribute__((noinline))
int overlapping_loop_patterns(int init) {
    int sum = init;
    
    // These three loops are at same nesting level
    // and will have partially overlapping block bitmaps
    
    // Loop G: Simple counted loop
    for (int a = 0; a < 24; ++a) {
        sum += a * 2;
        if (a % 6 == 0) {
            sum -= a;
        }
    }
    
    // Loop H: Shares some structure with G but different
    // Should intersect but not be subset
    int b = 0;
    while (b < 24) {
        sum += b * 3;
        if (b % 4 == 0) {
            sum += rand() % 5;
            b += 2;
            continue;
        }
        b++;
    }
    
    // Loop I: Different bounds, partially overlaps with G and H
    for (int c = 5; c < 20; c += 2) {
        sum ^= c;
        switch (c % 3) {
            case 0: sum += 10; break;
            case 1: sum += 20; break;
            case 2: sum += 30; break;
        }
    }
    
    return sum;
}

int main() {
    volatile int seed = 42; // Volatile to prevent constant folding
    
    // Create data dependencies between loops
    int result1 = complex_loops_1(seed);
    int result2 = complex_loops_2(result1);
    int result3 = overlapping_loop_patterns(result2);
    
    // Combine all results to ensure all loops are live
    int final_checksum = result1 ^ result2 ^ result3;
    
    // Use results to prevent dead code elimination
    printf("Loop analysis test checksum: %d\n", final_checksum);
    printf("Individual results: %d, %d, %d\n", result1, result2, result3);
    
    return final_checksum != 0 ? 0 : 1;
}

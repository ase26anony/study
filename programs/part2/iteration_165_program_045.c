#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 16; // volatile to prevent constant propagation
    int sum = 0;
    
    // Loop A: will have its own basic blocks
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        // Nested loop B: subset of A's blocks
        for (int j = 0; j < 8; ++j) {
            sum += (i * j) / 2;
            if (j == 4) break; // early exit creates additional blocks
        }
    }
    
    // Loop C: sequential to A, shares some blocks (post-loop code)
    int k = 0;
    while (k < 10) {
        sum += k * 3;
        if (k == 5) continue; // skip iteration
        k++;
    }
    
    return sum;
}

__attribute__((noinline))
int complex_loops_2(int base) {
    int result = base;
    volatile int outer_limit = 12;
    
    // Loop D: outer loop with complex control flow
    for (int x = 0; x < outer_limit; ++x) {
        switch (x % 4) {
            case 0: result += x * 2; break;
            case 1: result += x * 3; break;
            case 2: result += x * 4; break;
            default: result += x; break;
        }
        
        // Loop E: inner loop with early continue
        for (int y = 0; y < 6; ++y) {
            if (y % 2 == 0) continue;
            result += x * y;
            
            // Loop F: deeply nested, strict subset
            int z = 0;
            do {
                result += z;
                z++;
            } while (z < 3);
        }
    }
    
    // Loop G: sequential to D, partially overlapping blocks
    // (shares some control flow structures)
    int w = 0;
    while (w < 8) {
        result -= w * 2;
        w++;
        if (w == 4) {
            result += 100; // additional block
        }
    }
    
    return result;
}

__attribute__((noinline))
int overlapping_loop_patterns(int init) {
    int acc = init;
    volatile int mod = 7;
    
    // Loop H and I: sequential loops that will have partially overlapping
    // basic block bitmaps due to similar structure
    for (int a = 0; a < 20; a += 2) {
        acc += a * mod;
        if (a > 10) {
            acc -= 5;
        }
    }
    
    // Loop I: similar but not identical structure to H
    for (int b = 1; b < 20; b += 2) {
        acc += b * (mod - 1);
        if (b < 10) {
            acc += 3;
        } else {
            // Different block than H's else path
            acc -= 2;
        }
    }
    
    // Loop J: contains Loop K as strict subset
    for (int c = 0; c < 15; ++c) {
        acc += c * c;
        
        // Loop K: all blocks are subset of J's blocks
        for (int d = 0; d < 5; ++d) {
            acc += d;
            if (d == 2) {
                acc += 10; // creates additional block inside subset
            }
        }
        
        if (c == 7) {
            // Additional block in J but not in K
            acc += 50;
        }
    }
    
    return acc;
}

int main() {
    // Initialize with volatile to prevent compile-time computation
    volatile int seed = 42;
    
    // Execute all loop patterns with data dependencies
    int result1 = complex_loops_1(seed);
    int result2 = complex_loops_2(result1);
    int final_result = overlapping_loop_patterns(result2);
    
    // Use results to prevent dead code elimination
    printf("Final checksum: %d\n", final_result);
    
    // Additional volatile store to ensure all loops execute
    volatile int sink = final_result;
    
    return 0;
}

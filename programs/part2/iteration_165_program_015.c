#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 32; // Prevent constant propagation
    int sum = 0;
    
    // Loop A: Sequential loop that will share blocks with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        // Loop B: Nested inside A but with early exit
        // This creates partial block overlap with A
        for (int j = 0; j < 16; ++j) {
            if (j == i % 8) {
                sum += j * 3;
                break; // Early exit creates distinct block structure
            }
            sum += j;
        }
        
        // Conditional continue creates additional blocks
        if (i % 5 == 0) {
            continue;
        }
        sum += 1;
    }
    
    return sum;
}

__attribute__((noinline))
int complex_loops_2(int seed) {
    volatile int outer_limit = 24;
    volatile int inner_limit = 12;
    int sum = seed;
    
    // Loop C: Outer loop with switch inside
    for (int x = 0; x < outer_limit; ++x) {
        switch (x % 4) {
            case 0:
                sum += x * 2;
                break;
            case 1:
                sum += x * 3;
                // Loop D: Inner loop with subset of C's blocks
                for (int y = 0; y < inner_limit; ++y) {
                    if (y % 2 == 0) {
                        sum += y;
                        continue;
                    }
                    sum += y * 2;
                }
                break;
            case 2:
                sum += x * 4;
                // Loop E: Another inner loop with different structure
                do {
                    sum += x;
                    inner_limit--; // Modifies loop bound
                } while (inner_limit > 0);
                inner_limit = 12; // Reset
                break;
            default:
                sum += x;
        }
        
        // Shared post-loop block with sequential Loop F
        if (x % 3 == 0) {
            sum += 100;
        }
    }
    
    // Loop F: Sequential loop sharing header with Loop C
    // Creates bitmap intersection but not subset relationship
    for (int z = 0; z < 16; ++z) {
        if (z % 2 == 0) {
            // Function call creates additional basic blocks
            sum += complex_loops_1(z) % 100;
        } else {
            sum += z * 7;
        }
    }
    
    return sum;
}

__attribute__((noinline))
int overlapping_loop_patterns(int base) {
    int result = base;
    volatile int mod = 8;
    
    // Multiple sequential loops at same nesting level
    // Loop G
    for (int a = 0; a < 20; a++) {
        result += a;
        if (a == 10) {
            // Loop H: Strict subset of G's blocks
            for (int b = 0; b < 5; b++) {
                result += b * 2;
                if (b == 3) continue;
                result += 1;
            }
        }
    }
    
    // Loop I: Shares some blocks with G but not all
    for (int c = 0; c < 15; c++) {
        if (c % mod == 0) {
            result += c * 3;
        } else {
            result += c;
        }
        
        // Loop J: Deeply nested creating complex bitmap
        for (int d = 0; d < 3; d++) {
            for (int e = 0; e < 2; e++) {
                result += d * e;
                if (d == e) break;
            }
        }
    }
    
    return result;
}

__attribute__((noinline))
int hardware_loop_candidate(int iterations) {
    volatile int bound = iterations;
    int acc = 0;
    
    // Perfect hardware loop candidate with known iteration count
    // Loop K: Should be recognized by hw-doloop pass
    for (int i = 0; i < 64; i++) {
        acc += i * i;
        
        // Small inner loop with compile-time bounds
        // Loop L: Subset of K's blocks
        for (int j = 0; j < 8; j++) {
            acc += j;
            if (j == 4) continue;
            acc += 1;
        }
    }
    
    // Another hardware loop candidate with volatile bound
    // Loop M: Partially overlaps with N
    int count = 0;
    while (count < bound) {
        acc += count * 3;
        count++;
        
        // Conditional inner loop
        if (count % 10 == 0) {
            // Loop N: Intersects with M but not subset
            for (int k = 0; k < 5; k++) {
                acc += k * 7;
                if (k == 2) break;
            }
        }
    }
    
    return acc;
}

int main() {
    volatile int seed = 42; // Prevent constant folding
    int checksum = 0;
    
    // Execute all loop patterns to force analysis
    checksum += complex_loops_1(seed);
    checksum += complex_loops_2(checksum);
    checksum += overlapping_loop_patterns(checksum);
    checksum += hardware_loop_candidate(checksum % 100);
    
    // Additional sequential loops to create more overlap opportunities
    for (int final_i = 0; final_i < 10; final_i++) {
        checksum += final_i;
        // One more nested loop
        for (int final_j = 0; final_j < 3; final_j++) {
            checksum += final_j * 2;
            if (final_j == 1) continue;
            checksum += 5;
        }
    }
    
    // Ensure loops have observable effect
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

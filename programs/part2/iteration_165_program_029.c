#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(volatile int seed) {
    int sum = 0;
    volatile int limit = seed + 10;
    
    // Loop A: Sequential loop that will share blocks with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        // Loop B: Nested inside A, subset of A's blocks
        for (int j = 0; j < 5; ++j) {
            if (j % 2 == 0) {
                sum += j * 3;
                if (j == 2) break; // Early exit creates complex CFG
            }
        }
        
        if (i == limit / 2) {
            continue; // Skip some iterations
        }
    }
    
    // Loop C: Sequential to A, partially overlapping blocks
    int k = 0;
    while (k < limit) {
        switch (k % 4) {
            case 0: sum += k * 5; break;
            case 1: sum += k * 3; break;
            case 2: sum += k * 2; break;
            default: sum += k; break;
        }
        k++;
        if (k > limit / 2) {
            // Early continue creates shared post-loop block
            continue;
        }
    }
    
    return sum;
}

__attribute__((noinline))
int complex_loops_2(volatile int base) {
    int prod = 1;
    volatile int iterations = base % 8 + 4;
    
    // Loop D: do-while with complex exit conditions
    int m = 0;
    do {
        if (m % 2 == 0) {
            prod *= (m + 1);
        } else {
            prod *= (m + 2);
        }
        
        // Loop E: Strict subset of D's blocks
        for (int n = 0; n < 3; ++n) {
            prod += n;
            if (n == 1) {
                prod -= 2;
            }
        }
        
        m++;
        if (m > iterations) break;
    } while (m < iterations * 2); // Complex condition
    
    // Loop F: Sequential to D, shares header block
    for (int p = 0; p < iterations; ++p) {
        prod += p * p;
        if (p == iterations - 1) {
            // Creates shared exit block with Loop D
            prod *= 2;
        }
    }
    
    return prod;
}

__attribute__((noinline))
int overlapping_nested_loops(volatile int param) {
    int acc = param;
    
    // Outer loop G
    for (int x = 0; x < 16; ++x) {
        acc += x;
        
        // Multiple inner loops at same level
        // Loop H: subset of G
        for (int y = 0; y < 8; ++y) {
            acc += y * x;
            if (y == 4) continue;
        }
        
        // Loop I: also subset of G, different structure
        int z = 0;
        while (z < 4) {
            acc -= z;
            z++;
            if (z == 2) {
                acc += 10;
            }
        }
        
        // Conditional inner loop J
        if (x % 3 == 0) {
            for (int w = 0; w < 3; ++w) {
                acc *= (w + 1);
            }
        }
    }
    
    // Loop K: Sequential to G, partially overlapping
    for (int a = 0; a < 12; ++a) {
        acc += a * a;
        if (a == 6) {
            // Shared block with Loop G's exit
            acc /= 2;
        }
    }
    
    return acc;
}

__attribute__((noinline))
int hardware_loop_candidate(volatile int start) {
    int result = 0;
    
    // Perfect hardware loop candidate: fixed bounds, simple increment
    for (int i = start; i < start + 32; ++i) {
        result += i * i;
    }
    
    // Another candidate with moderate iteration count
    for (int j = 0; j < 64; j += 2) {
        result -= j;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int checksum = 0;
    
    // Execute all loop patterns with data dependencies
    int r1 = complex_loops_1(seed);
    checksum ^= r1;
    
    int r2 = complex_loops_2(seed + 1);
    checksum ^= r2;
    
    int r3 = overlapping_nested_loops(seed + 2);
    checksum ^= r3;
    
    int r4 = hardware_loop_candidate(seed + 3);
    checksum ^= r4;
    
    // Force all loops to be live through data flow
    checksum += r1 + r2 * 2 + r3 * 3 + r4 * 4;
    
    printf("Final checksum: %d\n", checksum);
    printf("Results: %d, %d, %d, %d\n", r1, r2, r3, r4);
    
    return checksum != 0 ? 0 : 1;
}

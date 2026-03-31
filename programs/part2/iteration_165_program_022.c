#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int bound1 = seed + 10;
    volatile int bound2 = seed + 5;
    int sum = 0;
    
    // Outer loop with complex control flow
    for (int i = 0; i < bound1; ++i) {
        // First inner loop - will be subset of outer loop's blocks
        for (int j = 0; j < bound2; ++j) {
            if (j % 3 == 0) {
                sum += i * j;
                continue;  // Creates additional basic blocks
            } else if (j % 5 == 0) {
                sum -= i;
                break;     // Early exit creates more blocks
            }
            sum += j;
        }
        
        // Conditional block inside outer loop
        if (i % 7 == 0) {
            sum *= 2;
        } else {
            sum += i;
        }
    }
    
    // Sequential loop at same level - shares some blocks with previous loops
    int k = 0;
    while (k < bound2) {
        switch (k % 4) {
            case 0: sum += 1; break;
            case 1: sum -= 2; break;
            case 2: sum *= 3; break;
            case 3: sum /= 4; break;
        }
        k++;
    }
    
    return sum;
}

__attribute__((noinline))
int complex_loops_2(int seed) {
    volatile int iterations = 32;  // Hardware loop candidate
    int result = seed;
    
    // Loop with compile-time known bound (32 iterations)
    for (int i = 0; i < iterations; ++i) {
        // Nested loop with partial overlap
        for (int j = i; j < iterations; ++j) {
            result += (i * j) % 17;
            
            // Conditional creates branching
            if (result > 1000) {
                result -= 500;
                continue;
            }
        }
        
        // Another loop at same nesting level
        int k = 0;
        do {
            result ^= (i << k);
            k++;
            
            // Function call inside loop
            if (k % 8 == 0) {
                result = abs(result);  // Creates call block
            }
        } while (k < 8);
    }
    
    // Post-loop block that might be shared
    if (result < 0) {
        result = -result;
    }
    
    return result;
}

__attribute__((noinline))
int overlapping_control_flow(int base) {
    int acc = base;
    volatile int limit1 = 20;
    volatile int limit2 = 15;
    
    // Two sequential loops that share header/post-loop blocks
    for (int x = 0; x < limit1; x++) {
        if (x % 2 == 0) {
            acc += x * 2;
        } else {
            acc -= x;
        }
        
        // Early exit creates additional blocks
        if (acc > 10000) break;
    }
    
    // Second loop at same level - shares some control flow blocks
    for (int y = 0; y < limit2; y++) {
        // Nested loop that's a strict subset
        for (int z = 0; z < 3; z++) {
            acc += y * z;
            
            // Multiple continues/breaks create complex CFG
            if (z == 1) continue;
            if (y == 10 && z == 2) break;
        }
        
        // Shared conditional structure
        if (acc % 2 == 0) {
            acc >>= 1;
        }
    }
    
    return acc;
}

__attribute__((noinline))
int hardware_loop_candidate() {
    int sum = 0;
    
    // Ideal hardware loop: fixed, moderate iteration count
    for (int i = 0; i < 64; i++) {
        // Simple arithmetic to prevent dead code elimination
        sum += i * i;
        
        // Minimal control flow to keep loop countable
        if (i % 16 == 0) {
            sum -= i;
        }
    }
    
    return sum;
}

int main() {
    volatile int seed = 42;  // Prevent constant propagation
    int checksum = 0;
    
    // Execute all loop patterns with data dependencies
    checksum += complex_loops_1(seed);
    checksum ^= complex_loops_2(checksum);
    checksum *= overlapping_control_flow(checksum % 100);
    checksum += hardware_loop_candidate();
    
    // Final observable output
    printf("Final checksum: %d\n", checksum);
    
    // Additional loops in main to create more opportunities
    volatile int final_iter = 8;
    for (int i = 0; i < final_iter; i++) {
        // Switch inside loop creates multiple blocks
        switch (i % 3) {
            case 0: checksum += i; break;
            case 1: checksum -= i * 2; break;
            case 2: checksum ^= checksum; break;
        }
    }
    
    printf("Final result: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;  // Ensure all loops affect result
}

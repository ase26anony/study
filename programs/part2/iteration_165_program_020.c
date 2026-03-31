#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 16; // volatile to prevent constant propagation
    int sum = 0;
    
    // Loop A: Will share some blocks with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
            continue; // Creates additional basic block
        } else if (i % 5 == 0) {
            sum += i * 3;
            break; // Early exit creates different block structure
        }
        sum += i;
    }
    
    // Loop B: Sequential loop that overlaps with Loop A's blocks
    // but not completely (partial overlap scenario)
    for (int j = 0; j < limit * 2; ++j) {
        switch (j % 4) {
            case 0:
                sum += j + seed;
                break;
            case 1:
                sum += j * seed;
                // fall through
            case 2:
                sum += 1;
                break;
            default:
                if (j > limit) {
                    sum -= 1;
                }
        }
    }
    
    return sum;
}

__attribute__((noinline))
int nested_loops_2(int base) {
    int total = 0;
    volatile int outer_bound = 8;
    
    // Outer loop: blocks will be superset of inner loop's blocks
    for (int x = 0; x < outer_bound; ++x) {
        // Inner loop 1: Strict subset of outer loop's blocks
        for (int y = 0; y < 4; ++y) {
            total += (x * y) + base;
            if (x + y > 10) {
                total -= 5; // Conditional creates additional block
            }
        }
        
        // Inner loop 2: Another subset, different structure
        int z = 0;
        while (z < 3) {
            total += z * x;
            if (z == 1) {
                total += base;
                continue; // Creates continue block
            }
            z++;
        }
        
        // Function call inside loop creates complex CFG
        total += complex_loops_1(x);
    }
    
    return total;
}

__attribute__((noinline))
int overlapping_loops_3(int init) {
    int result = init;
    volatile int n = 12;
    
    // Loop C and D will have partially overlapping blocks
    // due to shared control structures
    
    // Loop C
    int i = 0;
    do {
        result += i * i;
        if (i == n/2) {
            result += 100; // Special block in middle
        }
        i++;
    } while (i < n);
    
    // Loop D: Shares header/post-loop blocks but different body
    for (int j = 0; j < n; ++j) {
        if (j % 2 == 0) {
            result += j * 3;
            continue;
        }
        result += j * 7;
        
        // Nested conditional that creates block overlap with Loop C
        if (j > n/2) {
            result -= 50;
        }
    }
    
    return result;
}

__attribute__((noinline))
int hardware_loop_candidate(int iterations) {
    int acc = 0;
    
    // Perfect candidate for hardware loop: fixed bounds, simple IV
    // This creates a loop where blocks are subset of outer context
    for (int i = 0; i < 32; ++i) {
        acc += i * iterations;
        
        // Small conditional to create multiple blocks
        if (i & 1) {
            acc += 1;
        }
    }
    
    // Another countable loop at same level
    for (int j = 16; j > 0; --j) {
        acc += j * j;
    }
    
    return acc;
}

int main() {
    volatile int seed = 42; // volatile to prevent constant folding
    int checksum = 0;
    
    // Execute all loop patterns to ensure analysis
    checksum += complex_loops_1(seed);
    checksum += nested_loops_2(seed);
    checksum += overlapping_loops_3(checksum);
    checksum += hardware_loop_candidate(seed);
    
    // Additional sequential loops with shared blocks
    for (int k = 0; k < 20; ++k) {
        checksum += k;
        if (k == 10) {
            // Nested loop inside sequential loop
            for (int m = 0; m < 5; ++m) {
                checksum += m * 2;
            }
        }
    }
    
    // Final observable output
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

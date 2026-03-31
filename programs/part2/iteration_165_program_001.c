#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(volatile int seed) {
    int sum = 0;
    volatile int bound1 = seed % 10 + 5;
    volatile int bound2 = seed % 8 + 3;
    
    // Sequential loops at same nesting level that may share blocks
    for (int i = 0; i < bound1; ++i) {
        if (i % 2 == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        // Early continue creates additional basic blocks
        if (i == bound1 / 2) continue;
    }
    
    // Another loop at same level - may share exit blocks
    for (int j = 0; j < bound2; ++j) {
        switch (j % 3) {
            case 0: sum += j * 3; break;
            case 1: sum += j * 2; break;
            case 2: sum += j; break;
        }
        if (j > bound2 / 2) break;
    }
    
    return sum;
}

__attribute__((noinline))
int nested_loops_2(volatile int seed) {
    int total = 0;
    volatile int outer_bound = seed % 7 + 4;
    volatile int inner_bound = seed % 5 + 2;
    
    // Outer loop containing inner loop (strict subset relationship)
    for (int x = 0; x < outer_bound; ++x) {
        total += x * 10;
        
        // Inner loop - blocks are subset of outer loop's blocks
        for (int y = 0; y < inner_bound; ++y) {
            if (y % 2 == 0) {
                total += y;
            } else {
                total -= y;
            }
            // Function call creates additional basic block
            if (y == inner_bound - 1) {
                total += 100;
            }
        }
        
        // Conditional break in outer loop
        if (x > outer_bound / 2 && total > 500) {
            break;
        }
    }
    
    return total;
}

__attribute__((noinline))
int overlapping_loops_3(volatile int seed) {
    int acc = 0;
    volatile int limit = seed % 12 + 8;
    
    // First loop with complex control flow
    int i = 0;
    do {
        acc += i * i;
        if (i % 3 == 0) {
            acc += 5;
            // Nested conditional
            if (i > limit / 2) {
                acc += 10;
            }
        }
        i++;
    } while (i < limit);
    
    // Second loop that shares some blocks with first
    for (int j = 0; j < limit; ++j) {
        // Shared arithmetic pattern with first loop
        acc -= j * j;
        // Different control flow creates partial overlap
        switch (j % 4) {
            case 0: acc += 1; break;
            case 1: acc += 2; break;
            case 2: acc += 3; break;
            case 3: acc += 4; break;
        }
        // Early exit creates unique block
        if (acc < -1000) break;
    }
    
    return acc;
}

__attribute__((noinline))
int hardware_loop_candidate(volatile int seed) {
    int result = 0;
    // Fixed bound - good candidate for hardware loop
    for (int i = 0; i < 32; ++i) {
        result += i * 3;
        // Simple conditional to create basic blocks
        if (i & 1) {
            result += 7;
        }
    }
    
    // Another countable loop
    volatile int count = seed % 16 + 16;
    for (int j = count; j > 0; --j) {
        result -= j * 2;
    }
    
    return result;
}

__attribute__((noinline))
int mixed_loop_patterns(volatile int seed) {
    int val = seed;
    
    // While loop with complex condition
    while (val > 0) {
        val /= 2;
        // For loop inside while
        for (int k = 0; k < 8; ++k) {
            val += k;
            if (k == 4) continue;
            val -= 1;
        }
        
        // Do-while inside the same while
        int m = 0;
        do {
            val += m * m;
            m++;
            if (m > 5) break;
        } while (m < 10);
    }
    
    return val;
}

int main() {
    volatile int seed = 42; // Use volatile to prevent constant propagation
    int checksum = 0;
    
    // Execute all loop patterns, creating data dependencies
    checksum += complex_loops_1(seed);
    checksum += nested_loops_2(seed + 1);
    checksum += overlapping_loops_3(seed + 2);
    checksum += hardware_loop_candidate(seed + 3);
    checksum += mixed_loop_patterns(seed + 4);
    
    // Additional sequential loops in main
    for (int a = 0; a < 20; ++a) {
        checksum += a;
        if (a == 10) {
            // Nested loop creating subset relationship
            for (int b = 0; b < 5; ++b) {
                checksum -= b * 2;
            }
        }
    }
    
    // Another loop that may overlap with previous
    int temp = 0;
    for (int c = 0; c < 15; ++c) {
        temp += c * c;
        if (c % 2 == 0) {
            checksum += temp;
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1; // Ensure loops have effect
}

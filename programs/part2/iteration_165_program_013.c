#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 16; // Volatile to prevent constant propagation
    int sum = 0;
    
    // Loop A: Sequential loop that will share blocks with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        // Early continue creates additional basic blocks
        if (i == 7) continue;
        
        // Nested loop inside A - strict subset of A's blocks
        for (int j = 0; j < 8; ++j) {
            sum += j * i;
            if (j == 3) break; // Early exit creates more blocks
        }
    }
    
    // Loop B: Sequential to A, partially overlapping blocks
    // Shares the function's entry/exit blocks with A
    int count = limit / 2;
    while (count > 0) {
        sum += count * 3;
        if (count % 2 == 0) {
            // Another nested loop - subset relationship
            do {
                sum -= 1;
                count--; // Affects outer loop counter
            } while (count > limit / 4);
        }
        count--;
    }
    
    return sum;
}

__attribute__((noinline))
int complex_loops_2(int base) {
    int result = base;
    volatile int outer = 12; // Volatile bound
    
    // Loop C: Outer loop with switch inside
    for (int k = 0; k < outer; ++k) {
        switch (k % 4) {
            case 0: result += k * 2; break;
            case 1: result += k * 3; break;
            case 2: result += k * 4; break;
            default: result += k; break;
        }
        
        // Loop D: Inner loop with complex exit condition
        // Blocks are subset of Loop C's blocks
        int m = 0;
        while (m < 6) {
            result ^= (m << 2);
            if (result > 1000) break;
            m++;
        }
        
        // Loop E: Another inner loop at same level as D
        // Partially overlaps with D but not identical
        for (int n = 0; n < 4; ++n) {
            result += n * k;
            if (n == 2) continue;
            result -= 1;
        }
    }
    
    // Loop F: Sequential to Loop C, shares function exit block
    int temp = result;
    for (int p = 0; p < 8; ++p) {
        temp += complex_loops_1(p); // Function call creates new blocks
        if (p == 4) {
            // Early return path
            return temp % 1000;
        }
    }
    
    return result + temp;
}

__attribute__((noinline))
int overlapping_loop_patterns(int init) {
    int acc = init;
    
    // These three loops are sequential but share common setup/teardown blocks
    // Loop G
    for (int x = 0; x < 32; ++x) {
        acc += x * x;
        if (x == 16) {
            // This creates a branch block shared with other loops
            acc /= 2;
        }
    }
    
    // Loop H: Shares some blocks with G but not all
    int y = 0;
    while (y < 24) {
        acc -= y;
        if (acc < 0) {
            acc = 0;
            break; // Different exit path than G
        }
        y++;
    }
    
    // Loop I: Different structure but shares function epilogue
    do {
        acc += 5;
        if (acc > 100) {
            // Nested loop J: Strict subset of I's blocks
            for (int z = 0; z < 3; ++z) {
                acc <<= 1;
            }
            break;
        }
    } while (acc < 200);
    
    return acc;
}

int main() {
    // Initialize with volatile to prevent compile-time computation
    volatile int seed = 42;
    int checksum = 0;
    
    // Execute all loop patterns with data dependencies
    checksum += complex_loops_1(seed);
    checksum += complex_loops_2(checksum);
    checksum += overlapping_loop_patterns(checksum);
    
    // Additional sequential loops in main
    // Loop K: Hardware loop candidate with known bound
    for (int i = 0; i < 64; ++i) {
        checksum += i * i;
        // Loop L: Nested, subset relationship
        if (i % 8 == 0) {
            for (int j = 0; j < 8; ++j) {
                checksum ^= j;
            }
        }
    }
    
    // Loop M: Another sequential loop
    int counter = 100;
    while (counter--) {
        checksum += counter;
        if (checksum % 7 == 0) {
            continue; // Creates additional basic block
        }
    }
    
    // Final observable output
    printf("Final checksum: %d\n", checksum % 1000000);
    
    return 0;
}

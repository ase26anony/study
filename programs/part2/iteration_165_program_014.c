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
    }
    
    // Loop B: Shares some blocks with Loop A but has different structure
    // This creates partial bitmap overlap
    int j = 0;
    while (j < limit) {
        switch (j % 4) {
            case 0: sum += j * 3; break;
            case 1: sum += j * 2; break;
            case 2: sum += j; break;
            default: sum += 1; break;
        }
        j++;
    }
    
    // Loop C: Nested inside outer loop D
    // This creates subset relationship (C's blocks ⊆ D's blocks)
    for (int outer = 0; outer < 8; ++outer) {
        sum += outer * 10;
        
        // Inner loop - strict subset of outer loop's blocks
        for (int inner = 0; inner < 4; ++inner) {
            if (inner % 2 == 0) {
                sum += inner * 5;
            } else {
                sum += inner * 3;
            }
        }
        
        // Early exit that creates additional basic blocks
        if (outer == 5) {
            sum += 100;
            continue;
        }
    }
    
    return sum + seed;
}

__attribute__((noinline))
int complex_loops_2(int base) {
    int result = base;
    volatile int mod = 12; // Volatile to prevent optimization
    
    // Loop E: do-while with complex exit conditions
    // Creates overlapping but not subset relationship with Loop F
    int k = 0;
    do {
        result += k * k;
        
        if (k == mod / 2) {
            result -= 50; // Creates additional basic block
        }
        
        k++;
        
        // Complex condition to create multiple basic blocks
        if (k > 8 && result < 1000) {
            result += 200;
        }
    } while (k < mod);
    
    // Loop F: Sequential to Loop E, shares some blocks
    // This creates bitmap_intersect_p = true but not subset
    for (int m = 0; m < mod; m += 2) {
        result += m * 7;
        
        // Nested switch creates multiple basic blocks
        switch (m % 3) {
            case 0: result += 1; break;
            case 1: result += 2; 
                    if (m == 4) break; // Additional edge case
                    result += 3; 
                    break;
            case 2: result += 4; break;
        }
    }
    
    // Loop G: Hardware loop candidate with known iteration count
    // Should be recognized by hw-doloop pass
    for (int n = 0; n < 32; ++n) {
        result += n * n * n; // Non-trivial computation
    }
    
    return result;
}

__attribute__((noinline))
int overlapping_loop_pattern(int init) {
    int acc = init;
    
    // Create three sequential loops that share a common header block
    // through function call or shared setup code
    
    // Loop H
    for (int x = 0; x < 10; ++x) {
        acc += x * x;
        if (x == 5) {
            acc += complex_loops_1(1); // Function call creates shared block
        }
    }
    
    // Loop I - shares some blocks with H but not all
    for (int y = 5; y < 15; ++y) {
        acc -= y * 2;
        if (y == 10) {
            acc += complex_loops_1(2); // Same function call
        }
    }
    
    // Loop J - different structure but shares the function call block
    int z = 0;
    while (z < 8) {
        acc += z * 3;
        z++;
        if (z == 4) {
            acc += complex_loops_1(3); // Shared block
            continue;
        }
    }
    
    return acc;
}

int main() {
    volatile int seed = 42; // Volatile to prevent constant folding
    
    // Execute all loop patterns with data dependencies
    int result1 = complex_loops_1(seed);
    int result2 = complex_loops_2(result1);
    int final_result = overlapping_loop_pattern(result2);
    
    // Additional hardware loop candidates at top level
    int checksum = 0;
    
    // Hardware loop candidate with compile-time known bound
    for (int i = 0; i < 64; ++i) {
        checksum += i * i;
        if (i % 16 == 0) {
            checksum += final_result % 100; // Data dependency
        }
    }
    
    // Another candidate with different structure
    for (int j = 0; j < 48; j += 2) {
        checksum -= j * 3;
        switch (j % 5) {
            case 0: checksum += 1; break;
            case 1: checksum += 2; break;
            case 2: checksum += 3; break;
            case 3: checksum += 4; break;
            case 4: checksum += 5; break;
        }
    }
    
    // Final observable output
    printf("Final checksum: %d\n", checksum + final_result);
    
    return 0;
}

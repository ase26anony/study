#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(volatile int seed, int *shared_acc) {
    int acc = 0;
    
    // Loop A: Will have blocks that partially overlap with Loop B
    for (int i = 0; i < seed + 10; ++i) {
        if (i % 3 == 0) {
            acc += i * 2;
            // Early continue creates additional basic block
            continue;
        }
        
        // Loop B: Nested inside A's conditional path
        // Its blocks are a subset of Loop A's blocks
        for (int j = 0; j < 8; ++j) {
            if (j & 1) {
                acc += j * i;
            } else {
                acc -= j;
            }
        }
        
        // Function call creates another basic block
        if (i == seed) {
            break;  // Early exit creates another block
        }
    }
    
    // Post-loop block shared with other loops
    *shared_acc += acc;
    return acc;
}

__attribute__((noinline))
int complex_loops_2(volatile int bound, int *shared_acc) {
    int acc = 0;
    int i = 0;
    
    // Loop C: do-while with complex exit conditions
    // Shares some blocks with Loop A from complex_loops_1
    do {
        acc += i * 3;
        
        // Switch statement creates multiple basic blocks
        switch (i % 4) {
            case 0: acc += 1; break;
            case 1: acc += bound; break;
            case 2: continue;  // Jumps to loop header
            case 3: acc -= 5; break;
        }
        
        // Inner loop D: Strict subset of Loop C's blocks
        for (int k = 0; k < 5; ++k) {
            acc += k * k;
            if (k == 3) break;
        }
        
        i++;
    } while (i < bound && acc < 1000);
    
    // This post-loop block may be shared with other loops
    *shared_acc += acc;
    return acc;
}

__attribute__((noinline))
int sequential_loops(volatile int n, int *shared_acc) {
    int acc = 0;
    
    // Loop E: Simple countable loop
    // Will have some block overlap with Loop F
    for (int i = 0; i < n; ++i) {
        acc += i * i;
        if (i % 7 == 0) {
            continue;
        }
        acc += 1;
    }
    
    // Shared basic block between E and F
    
    // Loop F: Same nesting level as E, partially overlapping blocks
    for (int i = n - 1; i >= 0; --i) {
        acc -= i;
        if (i == n / 2) {
            break;  // Creates different exit block
        }
    }
    
    // Loop G: While loop at same level
    int j = 0;
    while (j < 5) {
        acc += j * 3;
        j++;
        if (j == 3) continue;
    }
    
    *shared_acc += acc;
    return acc;
}

__attribute__((noinline))
int nested_loop_system(volatile int base, int *shared_acc) {
    int acc = 0;
    
    // Outer loop H: Contains multiple inner loops
    for (int outer = 0; outer < base + 3; ++outer) {
        acc += outer;
        
        // Inner loop I: Strict subset of H's blocks
        for (int inner = 0; inner < 4; ++inner) {
            acc += inner * outer;
            if (inner == outer % 3) {
                continue;
            }
        }
        
        // Another inner loop J at same nesting level as I
        // Partially overlaps with I's blocks
        int k = 0;
        while (k < 3) {
            acc -= k;
            k++;
            if (k == 2) {
                // Nested break/continue creates complex flow
                break;
            }
        }
        
        // Conditional containing another loop K
        if (outer % 2 == 0) {
            // Loop K: Only executed conditionally
            for (int m = 0; m < 2; ++m) {
                acc += m * 7;
            }
        }
    }
    
    *shared_acc += acc;
    return acc;
}

int main() {
    // Use volatile to prevent constant propagation
    volatile int seed = 15;
    volatile int bound = 12;
    volatile int n = 20;
    volatile int base = 8;
    
    int shared_accumulator = 0;
    int result1, result2, result3, result4;
    
    // Execute all loop patterns
    result1 = complex_loops_1(seed, &shared_accumulator);
    result2 = complex_loops_2(bound, &shared_accumulator);
    result3 = sequential_loops(n, &shared_accumulator);
    result4 = nested_loop_system(base, &shared_accumulator);
    
    // Create data dependencies between results
    int final_checksum = result1 + result2 * 3 - result3 + result4 * 2;
    final_checksum += shared_accumulator;
    
    // Ensure all computations are used
    printf("Loop analysis test - Checksum: %d\n", final_checksum);
    printf("Individual results: %d, %d, %d, %d\n", 
           result1, result2, result3, result4);
    printf("Shared accumulator: %d\n", shared_accumulator);
    
    return final_checksum != 0 ? 0 : 1;
}

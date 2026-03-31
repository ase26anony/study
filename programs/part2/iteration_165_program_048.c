#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
#define NOINLINE __attribute__((noinline))

// Volatile variables to prevent optimization
static volatile int g_seed = 42;
static volatile int g_bound = 32;

// Function prototypes
NOINLINE int complex_loops_1(int start);
NOINLINE int complex_loops_2(int start);
NOINLINE int overlapping_loops(int base);
NOINLINE int nested_subset_loops(int iterations);
NOINLINE int sequential_shared_blocks(int init);

// Complex loop structure with partial overlap
NOINLINE int complex_loops_1(int start) {
    int acc1 = 0, acc2 = 0;
    volatile int limit = g_bound;
    
    // Loop A: for loop with conditional break
    for (int i = start; i < limit; ++i) {
        acc1 += i * 2;
        
        // Conditional that creates additional basic blocks
        if (i % 7 == 0) {
            acc1 -= 3;
            continue;
        }
        
        // Nested loop B: while loop (subset of Loop A's blocks)
        int j = 0;
        while (j < 5) {
            acc2 += i * j;
            j++;
            
            // Early exit creates more blocks
            if (acc2 > 1000) break;
        }
        
        // Switch creates multiple basic blocks
        switch (i % 4) {
            case 0: acc1 += 10; break;
            case 1: acc1 -= 5; break;
            case 2: acc1 *= 2; break;
            default: acc1 /= 2; break;
        }
    }
    
    // Loop C: do-while that shares some blocks with Loop A's exit path
    int k = 0;
    do {
        acc2 += k * 3;
        k++;
        
        if (k % 3 == 0) {
            // Function call creates new block
            acc2 += complex_loops_2(k);
        }
    } while (k < 10);
    
    return acc1 + acc2;
}

// Another complex function with overlapping loops
NOINLINE int complex_loops_2(int start) {
    int acc = start;
    volatile int mod = g_seed % 16 + 8;
    
    // Loop D: for loop with complex condition
    for (int i = 0; i < mod; i += 2) {
        acc += i * i;
        
        // Inner loop E: strict subset of Loop D's blocks
        for (int j = 0; j < 3; ++j) {
            acc += (i + j) * 2;
            
            // Conditional continue
            if ((i + j) % 2 == 0) {
                acc -= 1;
                continue;
            }
        }
        
        // Conditional break creates divergent paths
        if (acc > 500) {
            acc /= 2;
            break;
        }
    }
    
    // Loop F: while loop that shares header with Loop D's exit
    int counter = 0;
    while (counter < 5) {
        acc += counter * counter;
        counter++;
        
        // Nested switch
        switch (counter % 3) {
            case 0: acc += 7; break;
            case 1: acc -= 3; break;
            case 2: acc *= 1; break;
        }
    }
    
    return acc;
}

// Function with sequential loops sharing blocks
NOINLINE int overlapping_loops(int base) {
    int result = base;
    volatile int shared_limit = 20;
    
    // Loop G and H share the same initialization and some blocks
    
    // Loop G: for with if-else chain
    for (int i = 0; i < shared_limit; ++i) {
        result += i;
        
        if (i % 2 == 0) {
            result *= 2;
        } else if (i % 3 == 0) {
            result -= i;
        } else {
            result += 5;
        }
    }
    
    // Loop H: while loop that overlaps with Loop G's blocks
    int j = 0;
    while (j < shared_limit) {
        result -= j * 2;
        j += 2;
        
        // This if-else mimics structure from Loop G
        if (j % 4 == 0) {
            result += 10;
        }
    }
    
    // Loop I: do-while that's a subset of the previous loops' blocks
    int k = 5;
    do {
        result += k * k;
        k--;
        
        // Shared conditional structure
        if (k % 2 == 0) {
            result -= 3;
        }
    } while (k > 0);
    
    return result;
}

// Function with strictly nested loops (inner is subset of outer)
NOINLINE int nested_subset_loops(int iterations) {
    int total = 0;
    volatile int outer_bound = iterations;
    
    // Outer loop J: contains all blocks of inner loops
    for (int outer = 0; outer < outer_bound; ++outer) {
        total += outer * 10;
        
        // Multiple inner loops at same level
        // Inner loop K: subset of J's blocks
        for (int inner1 = 0; inner1 < 4; ++inner1) {
            total += outer * inner1;
            
            if (inner1 % 2 == 0) {
                total += 2;
                continue;
            }
        }
        
        // Inner loop L: another subset
        int inner2 = 0;
        while (inner2 < 3) {
            total -= outer + inner2;
            inner2++;
        }
        
        // Conditional that creates shared exit block
        if (outer % 5 == 0) {
            total /= 2;
        }
    }
    
    return total;
}

// Sequential loops with shared entry/exit blocks
NOINLINE int sequential_shared_blocks(int init) {
    int val = init;
    
    // Shared setup code
    volatile int common = g_seed % 10 + 5;
    
    // Loop M
    for (int i = 0; i < common; ++i) {
        val += i * i;
        
        // Shared conditional pattern
        if (val > 100) {
            val -= 50;
        }
    }
    
    // Loop N - shares exit blocks with M
    int j = common - 1;
    while (j >= 0) {
        val += j * 3;
        j--;
        
        // Same conditional pattern
        if (val > 100) {
            val -= 50;
        }
    }
    
    // Loop O - shares entry with previous loops' exit
    for (int k = 0; k < 8; ++k) {
        val += k;
        
        // Different conditional to create partial overlap
        if (k % 2 == 0) {
            val *= 2;
        } else {
            val -= 1;
        }
    }
    
    return val;
}

int main() {
    int final_result = 0;
    
    // Initialize with volatile to prevent constant propagation
    volatile int init_val = g_seed;
    
    // Execute all complex loop patterns
    final_result += complex_loops_1(init_val);
    final_result += complex_loops_2(final_result % 20);
    final_result += overlapping_loops(final_result);
    final_result += nested_subset_loops(g_bound / 2);
    final_result += sequential_shared_blocks(final_result % 100);
    
    // Additional sequential loops in main
    volatile int main_limit = 15;
    
    // Loop P
    for (int i = 0; i < main_limit; ++i) {
        final_result += i * i;
        
        // Nested loop Q (subset)
        for (int j = 0; j < 3; ++j) {
            final_result -= i * j;
        }
    }
    
    // Loop R (partially overlaps with P)
    int k = main_limit - 1;
    while (k >= 0) {
        final_result += k * 3;
        k -= 2;
        
        if (final_result % 2 == 0) {
            final_result /= 2;
        }
    }
    
    // Print result to prevent dead code elimination
    printf("Final checksum: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
